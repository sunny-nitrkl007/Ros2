#pragma once

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

class ObjectDetector : public rclcpp::Node
{
public:
    ObjectDetector();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr detection_group_;

    // Publishers
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr detected_objects_pub_;

    // Subscribers
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr fused_environment_sub_;
    void on_fused_environment(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

    // Parameters
    double confidence_threshold_;
    int64_t max_objects_;
    double detection_range_;
};