#pragma once

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"

class LidarProcessor : public rclcpp::Node
{
public:
    LidarProcessor();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr scan_group_;

    // Publishers
    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr obstacle_map_pub_;

    // Timers
    rclcpp::TimerBase::SharedPtr scan_tick_timer_;
    void scan_tick();

    // Parameters
    double scan_rate_;
    double max_range_;
    double obstacle_density_;

    // State
    uint64_t tick_    = 0;
    int      grid_sz_ = 200;
};