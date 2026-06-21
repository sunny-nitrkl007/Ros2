#include "safety_monitor.hpp"

using namespace std::chrono_literals;

SafetyMonitor::SafetyMonitor()
: Node("safety_monitor")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("check_rate", 50.0);
    check_rate_ = this->get_parameter("check_rate").as_double();
    this->declare_parameter<double>("min_safe_distance", 3.0);
    min_safe_distance_ = this->get_parameter("min_safe_distance").as_double();
    this->declare_parameter<double>("fault_timeout", 0.5);
    fault_timeout_ = this->get_parameter("fault_timeout").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    monitor_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(10));
        qos.lifespan(std::chrono::milliseconds(50));
        safety_status_pub_ = this->create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/adas/safety/safety_status", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /adas/safety/safety_status");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        detected_objects_sub_ = this->create_subscription<visualization_msgs::msg::MarkerArray>(
            "/adas/perception/detected_objects", qos,
            std::bind(&SafetyMonitor::on_detected_objects, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/perception/detected_objects");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        driving_command_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/adas/planning/driving_command", qos,
            std::bind(&SafetyMonitor::on_driving_command, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/planning/driving_command");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    emergency_stop_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/adas/safety/emergency_stop",
        std::bind(&SafetyMonitor::on_emergency_stop, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        service_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /adas/safety/emergency_stop");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 50.0));
        safety_check_timer_ = this->create_wall_timer(
            period,
            std::bind(&SafetyMonitor::safety_check, this),
            monitor_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'safety_check' started at 50.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyMonitor::on_detected_objects(const visualization_msgs::msg::MarkerArray::SharedPtr msg)
{
    //-- begin impl [on_detected_objects] ----------------------------------------
    nearest_distance_ = 999.0;
    for (const auto & m : msg->markers) {
        double dist = std::sqrt(m.pose.position.x * m.pose.position.x +
                                m.pose.position.y * m.pose.position.y);
        if (dist < nearest_distance_)
            nearest_distance_ = dist;
    }
    //-- end impl [on_detected_objects] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyMonitor::on_driving_command(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    //-- begin impl [on_driving_command] ----------------------------------------
    last_brake_ = -std::min(msg->linear.x, 0.0);  // negative linear.x = braking
    //-- end impl [on_driving_command] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyMonitor::on_emergency_stop(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_emergency_stop] ----------------------------------------
    (void)request;
    emergency_active_ = true;
    response->success = true;
    response->message = "Emergency stop acknowledged by safety_monitor";
    RCLCPP_ERROR(this->get_logger(), "EMERGENCY STOP triggered");
    //-- end impl [on_emergency_stop] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyMonitor::safety_check()
{
    //-- begin impl [safety_check] ----------------------------------------
    auto arr = diagnostic_msgs::msg::DiagnosticArray();
    arr.header.stamp = this->now();

    diagnostic_msgs::msg::DiagnosticStatus status;
    status.name = "proximity_check";

    if (emergency_active_) {
        status.level   = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
        status.message = "EMERGENCY STOP active";
        consecutive_faults_ = 0;
    } else if (nearest_distance_ < min_safe_distance_) {
        ++consecutive_faults_;
        if (consecutive_faults_ >= 3) {
            status.level   = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
            status.message = "COLLISION RISK: obstacle at " +
                             std::to_string(nearest_distance_).substr(0, 4) + "m";
            RCLCPP_WARN(this->get_logger(), "Safety fault: obstacle %.2fm (min %.2fm)",
                nearest_distance_, min_safe_distance_);
        } else {
            status.level   = diagnostic_msgs::msg::DiagnosticStatus::WARN;
            status.message = "Obstacle approaching";
        }
    } else {
        consecutive_faults_ = 0;
        status.level   = diagnostic_msgs::msg::DiagnosticStatus::OK;
        status.message = "Clear (nearest: " + std::to_string(nearest_distance_).substr(0, 4) + "m)";
    }

    arr.status.push_back(status);
    safety_status_pub_->publish(arr);
    //-- end impl [safety_check] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<SafetyMonitor>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}