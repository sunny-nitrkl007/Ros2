#include "bucket_controller.hpp"
#include <string>

using namespace std::chrono_literals;

BucketController::BucketController()
: Node("bucket_controller")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("max_load_kg", 250.0);
    max_load_kg_ = this->get_parameter("max_load_kg").as_double();
    this->declare_parameter<double>("fill_rate", 30.0);
    fill_rate_ = this->get_parameter("fill_rate").as_double();
    this->declare_parameter<double>("dump_threshold", 0.85);
    dump_threshold_ = this->get_parameter("dump_threshold").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    bucket_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        bucket_state_pub_ = this->create_publisher<geometry_msgs::msg::Vector3>("/mining/excavator/bucket_state", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/excavator/bucket_state");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = bucket_group_;
        dig_command_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "/mining/excavator/dig_command", qos,
            std::bind(&BucketController::on_dig_command, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/excavator/dig_command");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    check_payload_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/mining/excavator/check_payload",
        std::bind(&BucketController::on_check_payload, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        bucket_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /mining/excavator/check_payload");

}

// ─────────────────────────────────────────────────────────────────────────────
void BucketController::on_dig_command(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    //-- begin impl [on_dig_command] ----------------------------------------
    target_angle_ = msg->x;
    is_digging_   = (msg->y > 0.1);  // depth > 0 = actively digging

    // Smoothly track target angle
    double diff = target_angle_ - current_angle_;
    current_angle_ += diff * 0.15;

    // Accumulate material while digging (fill_rate kg/s, called ~5 Hz)
    if (is_digging_ && current_load_kg_ < max_load_kg_)
        current_load_kg_ += fill_rate_ * 0.2;

    // Publish state: x=angle, y=load_pct (0-1), z=depth
    auto state = geometry_msgs::msg::Vector3();
    state.x = current_angle_;
    state.y = current_load_kg_ / max_load_kg_;
    state.z = msg->y;
    bucket_state_pub_->publish(state);
    //-- end impl [on_dig_command] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void BucketController::on_check_payload(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_check_payload] ----------------------------------------
    (void)request;
    bool full = (current_load_kg_ >= dump_threshold_ * max_load_kg_);
    response->success = full;
    response->message = full
        ? "Bucket full: " + std::to_string(static_cast<int>(current_load_kg_)) + "kg"
        : "Loading: " + std::to_string(static_cast<int>(current_load_kg_)) + "/" +
          std::to_string(static_cast<int>(max_load_kg_)) + "kg";
    if (full) {
        RCLCPP_INFO(this->get_logger(), "Bucket dumped (%.0fkg) — resetting", current_load_kg_);
        current_load_kg_ = 0.0;
    }
    //-- end impl [on_check_payload] ------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BucketController>());
    rclcpp::shutdown();
    return 0;
}