#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

class TruckNavigator : public rclcpp::Node
{
public:
    TruckNavigator();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr nav_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr truck_pose_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr waypoint_command_sub_;
    void on_waypoint_command(const geometry_msgs::msg::PoseStamped::SharedPtr msg);

    // Timers
    rclcpp::TimerBase::SharedPtr navigation_tick_timer_;
    void navigation_tick();

    // Parameters
    double nav_rate_;
    double max_speed_;
    double position_tolerance_;

    // State
    double current_x_  = 100.0;
    double current_y_  = 50.0;
    double target_x_   = 100.0;
    double target_y_   = 50.0;
    bool   has_target_ = false;
    bool   at_target_  = true;
};