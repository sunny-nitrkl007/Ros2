#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/trigger.hpp"

class Watchdog : public rclcpp::Node
{
public:
    Watchdog();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr watch_group_;

    // Publishers
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr heartbeat_pub_;

    // Subscribers
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr safety_status_sub_;
    void on_safety_status(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr system_health_srv_;
    void on_system_health(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Timers
    rclcpp::TimerBase::SharedPtr heartbeat_tick_timer_;
    void heartbeat_tick();

    // Parameters
    double heartbeat_rate_;
    double timeout_threshold_;
};