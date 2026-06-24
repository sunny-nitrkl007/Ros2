#pragma once

#include "rclcpp/rclcpp.hpp"
#include "adas_interfaces/msg/emergency_brake.hpp"
#include "adas_interfaces/msg/obstacle_data.hpp"
#include "adas_interfaces/srv/get_lane_info.hpp"

class Perception1Node : public rclcpp::Node
{
public:
    Perception1Node();

private:
    // Publishers
    rclcpp::Publisher<adas_interfaces::msg::ObstacleData>::SharedPtr pub_obstacle_data_pub_;

    // Subscribers
    rclcpp::Subscription<adas_interfaces::msg::EmergencyBrake>::SharedPtr sub_emergency_state_sub_;
    void on_sub_emergency_state(const adas_interfaces::msg::EmergencyBrake::SharedPtr msg);

    // Service servers
    rclcpp::Service<adas_interfaces::srv::GetLaneInfo>::SharedPtr server_get_lane_info_srv_;
    void on_server_get_lane_info(
        const adas_interfaces::srv::GetLaneInfo::Request::SharedPtr request,
        adas_interfaces::srv::GetLaneInfo::Response::SharedPtr response);

    // Parameters
    double sensor_update_rate_hz_;
    double obstacle_detection_range_m_;
    bool enable_object_classification_;
    bool enable_debug_logging_;
};