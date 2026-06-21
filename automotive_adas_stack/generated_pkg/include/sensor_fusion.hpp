#pragma once

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"

class SensorFusion : public rclcpp::Node
{
public:
    SensorFusion();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr fusion_group_;

    // Publishers
    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr fused_environment_pub_;

    // Timers
    rclcpp::TimerBase::SharedPtr fusion_tick_timer_;
    void fusion_tick();

    // Parameters
    double lidar_range_;
    double fusion_rate_;
    double grid_resolution_;
};