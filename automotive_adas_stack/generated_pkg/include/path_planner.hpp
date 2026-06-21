#pragma once

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/path.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

class PathPlanner : public rclcpp::Node
{
public:
    PathPlanner();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr planning_group_;

    // Publishers
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr planned_path_pub_;

    // Subscribers
    rclcpp::Subscription<visualization_msgs::msg::MarkerArray>::SharedPtr detected_objects_sub_;
    void on_detected_objects(const visualization_msgs::msg::MarkerArray::SharedPtr msg);

    // Timers
    rclcpp::TimerBase::SharedPtr planning_cycle_timer_;
    void planning_cycle();

    // Parameters
    double planning_rate_;
    double look_ahead_distance_;
    double max_speed_;
};