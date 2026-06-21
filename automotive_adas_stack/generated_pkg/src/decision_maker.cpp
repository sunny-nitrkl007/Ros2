#include "decision_maker.hpp"

using namespace std::chrono_literals;

DecisionMaker::DecisionMaker()
: Node("decision_maker")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("target_speed", 10.0);
    target_speed_ = this->get_parameter("target_speed").as_double();
    this->declare_parameter<double>("brake_threshold", 5.0);
    brake_threshold_ = this->get_parameter("brake_threshold").as_double();
    this->declare_parameter<double>("steering_gain", 0.5);
    steering_gain_ = this->get_parameter("steering_gain").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    decision_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        driving_command_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/adas/planning/driving_command", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /adas/planning/driving_command");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = decision_group_;
        planned_path_sub_ = this->create_subscription<nav_msgs::msg::Path>(
            "/adas/planning/planned_path", qos,
            std::bind(&DecisionMaker::on_planned_path, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/planning/planned_path");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(10));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = decision_group_;
        safety_status_sub_ = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
            "/adas/safety/safety_status", qos,
            std::bind(&DecisionMaker::on_safety_status, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/safety/safety_status");
    }

    // ── Service clients ───────────────────────────────────────────────────────
    emergency_stop_client_ = this->create_client<std_srvs::srv::Trigger>("/adas/safety/emergency_stop");
    RCLCPP_INFO(this->get_logger(), "Client created: /adas/safety/emergency_stop");

}

// ─────────────────────────────────────────────────────────────────────────────
void DecisionMaker::on_planned_path(const nav_msgs::msg::Path::SharedPtr msg)
{
    //-- begin impl [on_planned_path] ----------------------------------------
    if (emergency_active_ || msg->poses.empty()) return;

    // Derive steering from first waypoint lateral offset
    double lateral = msg->poses.front().pose.position.y;
    current_steer_ = steering_gain_ * lateral;

    // Reduce speed if path curves sharply
    double speed_factor = 1.0 - std::min(std::abs(lateral) / 5.0, 0.6);
    current_speed_ = target_speed_ * speed_factor;

    auto cmd = geometry_msgs::msg::Twist();
    cmd.linear.x  = current_speed_;
    cmd.angular.z = current_steer_;
    driving_command_pub_->publish(cmd);
    //-- end impl [on_planned_path] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void DecisionMaker::on_safety_status(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg)
{
    //-- begin impl [on_safety_status] ----------------------------------------
    bool had_fault = safety_fault_;
    safety_fault_ = false;
    for (const auto & status : msg->status) {
        if (status.level >= diagnostic_msgs::msg::DiagnosticStatus::ERROR) {
            safety_fault_ = true;
            break;
        }
    }

    if (safety_fault_ && !emergency_active_) {
        emergency_active_ = true;
        RCLCPP_ERROR(this->get_logger(), "Safety fault — initiating emergency stop");

        // Publish zero command immediately
        driving_command_pub_->publish(geometry_msgs::msg::Twist());

        if (emergency_stop_client_->service_is_ready()) {
            auto req = std::make_shared<std_srvs::srv::Trigger::Request>();
            emergency_stop_client_->async_send_request(req,
                [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture f) {
                    RCLCPP_WARN(this->get_logger(), "Emergency stop ack: %s",
                        f.get()->message.c_str());
                });
        }
    } else if (!safety_fault_ && had_fault) {
        RCLCPP_INFO(this->get_logger(), "Safety cleared — resuming");
        emergency_active_ = false;
    }
    //-- end impl [on_safety_status] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<DecisionMaker>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}