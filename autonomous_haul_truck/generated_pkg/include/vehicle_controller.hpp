#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/path.hpp"
#include "std_srvs/srv/trigger.hpp"

class VehicleController : public rclcpp::Node
{
public:
    VehicleController();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr control_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr drive_command_pub_;

    // Subscribers
    rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr planned_route_sub_;
    void on_planned_route(const nav_msgs::msg::Path::SharedPtr msg);
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr hazard_alert_sub_;
    void on_hazard_alert(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    // Service clients
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr request_stop_client_;

    // Timers
    rclcpp::TimerBase::SharedPtr control_tick_timer_;
    void control_tick();

    // Parameters
    double max_speed_;
    double max_steering_;
    double control_rate_;
    double brake_decel_;

    // State
    double next_wp_x_     = 500.0;
    double next_wp_y_     = 0.0;
    bool   has_route_     = false;
    double current_speed_ = 0.0;
    bool   emergency_     = false;
    int    hazard_level_  = 0;
};