#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/path.hpp"
#include "std_srvs/srv/trigger.hpp"

class DecisionMaker : public rclcpp::Node
{
public:
    DecisionMaker();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr decision_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr driving_command_pub_;

    // Subscribers
    rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr planned_path_sub_;
    void on_planned_path(const nav_msgs::msg::Path::SharedPtr msg);
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr safety_status_sub_;
    void on_safety_status(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    // Service clients
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr emergency_stop_client_;

    // Parameters
    double target_speed_;
    double brake_threshold_;
    double steering_gain_;
};