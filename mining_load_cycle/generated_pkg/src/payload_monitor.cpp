#include "payload_monitor.hpp"
#include <algorithm>
#include <string>

using namespace std::chrono_literals;

PayloadMonitor::PayloadMonitor()
: Node("payload_monitor")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("target_payload_kg", 200.0);
    target_payload_kg_ = this->get_parameter("target_payload_kg").as_double();
    this->declare_parameter<double>("check_interval", 2.0);
    check_interval_ = this->get_parameter("check_interval").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    payload_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        payload_status_pub_ = this->create_publisher<geometry_msgs::msg::Vector3>("/mining/truck/payload_status", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/truck/payload_status");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = payload_group_;
        bucket_state_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "/mining/excavator/bucket_state", qos,
            std::bind(&PayloadMonitor::on_bucket_state, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/excavator/bucket_state");
    }

    // ── Service clients ───────────────────────────────────────────────────────
    check_payload_client_ = this->create_client<std_srvs::srv::Trigger>("/mining/excavator/check_payload");
    RCLCPP_INFO(this->get_logger(), "Client created: /mining/excavator/check_payload");

}

// ─────────────────────────────────────────────────────────────────────────────
void PayloadMonitor::on_bucket_state(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    //-- begin impl [on_bucket_state] ----------------------------------------
    double load_pct = msg->y;  // 0-1

    // Dump event: bucket was nearly full last tick, now near-empty (angle swung back)
    if (last_load_pct_ >= 0.8 && load_pct < 0.1) {
        double added_kg = last_load_pct_ * 30000.0;  // ~30t bucket at 395 class
        total_payload_kg_ += added_kg;
        payload_full_ = (total_payload_kg_ >= target_payload_kg_);
        RCLCPP_INFO(this->get_logger(), "Dump: +%.0fkg → truck total=%.0fkg / %.0fkg (%.0f%%)",
            added_kg, total_payload_kg_, target_payload_kg_,
            100.0 * total_payload_kg_ / target_payload_kg_);
        if (payload_full_) {
            RCLCPP_WARN(this->get_logger(), "Truck at capacity — ready to dispatch");
            // Optionally verify via service
            if (check_payload_client_->service_is_ready()) {
                auto req = std::make_shared<std_srvs::srv::Trigger::Request>();
                check_payload_client_->async_send_request(req,
                    [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture f) {
                        RCLCPP_INFO(this->get_logger(), "Bucket status: %s", f.get()->message.c_str());
                    });
            }
        }
    }
    last_load_pct_ = load_pct;

    auto status = geometry_msgs::msg::Vector3();
    status.x = total_payload_kg_;
    status.y = target_payload_kg_;
    status.z = std::min(1.0, total_payload_kg_ / target_payload_kg_);
    payload_status_pub_->publish(status);
    //-- end impl [on_bucket_state] ------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PayloadMonitor>());
    rclcpp::shutdown();
    return 0;
}