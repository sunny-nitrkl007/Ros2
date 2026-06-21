#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "std_msgs/msg/bool.hpp"

class ProximityDetector : public rclcpp::Node
{
public:
    ProximityDetector();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr prox_group_;

    // Publishers
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr proximity_alert_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr truck_pose_sub_;
    void on_truck_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg);

    // Timers
    rclcpp::TimerBase::SharedPtr proximity_check_timer_;
    void proximity_check();

    // Parameters
    double check_rate_;
    double proximity_radius_;
    double excavator_x_;
    double excavator_y_;

    // State
    double truck_x_    = 999.0;
    double truck_y_    = 999.0;
    bool   has_truck_  = false;
    bool   in_range_   = false;
};