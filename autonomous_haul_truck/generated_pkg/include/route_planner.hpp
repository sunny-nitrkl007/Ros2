#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/path.hpp"

class RoutePlanner : public rclcpp::Node
{
public:
    RoutePlanner();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr planning_group_;

    // Publishers
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr planned_route_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr vehicle_pose_sub_;
    void on_vehicle_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr obstacle_map_sub_;
    void on_obstacle_map(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

    // Timers
    rclcpp::TimerBase::SharedPtr planning_tick_timer_;
    void planning_tick();

    // Parameters
    double planning_rate_;
    double destination_x_;
    double destination_y_;
    double waypoint_spacing_;

    // State
    double current_x_     = 0.0;
    double current_y_     = 0.0;
    bool   has_pose_      = false;
    int    nearest_obs_m_ = 999;
};