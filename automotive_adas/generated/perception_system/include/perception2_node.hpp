#pragma once

#include "rclcpp/rclcpp.hpp"
#include "adas_interfaces/srv/request_path_update.hpp"

class Perception2Node : public rclcpp::Node
{
public:
    Perception2Node();

private:
    // Service clients
    rclcpp::Client<adas_interfaces::srv::RequestPathUpdate>::SharedPtr client_request_path_update_client_;

    // Parameters
    double sensor_update_rate_hz_;
    double obstacle_detection_range_m_;
    bool enable_object_classification_;
    bool enable_debug_logging_;
};