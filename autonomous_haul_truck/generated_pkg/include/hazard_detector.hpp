#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "std_srvs/srv/trigger.hpp"

class HazardDetector : public rclcpp::Node
{
public:
    HazardDetector();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr monitor_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr hazard_alert_pub_;

    // Subscribers
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr obstacle_map_sub_;
    void on_obstacle_map(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr drive_command_sub_;
    void on_drive_command(const geometry_msgs::msg::Twist::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr request_stop_srv_;
    void on_request_stop(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Timers
    rclcpp::TimerBase::SharedPtr hazard_check_timer_;
    void hazard_check();

    // Parameters
    double critical_distance_;
    double warning_distance_;
    double check_rate_;

    // State
    double nearest_dist_m_  = 999.0;
    double commanded_speed_ = 0.0;
    bool   emergency_active_ = false;
    int    consecutive_warn_ = 0;
};