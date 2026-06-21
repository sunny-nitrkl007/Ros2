#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

class SafetyMonitor : public rclcpp::Node
{
public:
    SafetyMonitor();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr monitor_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr safety_status_pub_;

    // Subscribers
    rclcpp::Subscription<visualization_msgs::msg::MarkerArray>::SharedPtr detected_objects_sub_;
    void on_detected_objects(const visualization_msgs::msg::MarkerArray::SharedPtr msg);
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr driving_command_sub_;
    void on_driving_command(const geometry_msgs::msg::Twist::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr emergency_stop_srv_;
    void on_emergency_stop(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Timers
    rclcpp::TimerBase::SharedPtr safety_check_timer_;
    void safety_check();

    // Parameters
    double check_rate_;
    double min_safe_distance_;
    double fault_timeout_;
};