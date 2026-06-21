#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

class GpsImuFusion : public rclcpp::Node
{
public:
    GpsImuFusion();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr fusion_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr vehicle_pose_pub_;

    // Timers
    rclcpp::TimerBase::SharedPtr fusion_tick_timer_;
    void fusion_tick();

    // Parameters
    double fusion_rate_;
    double position_noise_;
    double heading_drift_;

    // State
    uint64_t tick_   = 0;
    double   pos_x_  = 0.0;
    double   pos_y_  = 0.0;
    double   heading_ = 0.0;
};