#pragma once

#include "rclcpp/rclcpp.hpp"
#include "adas_interfaces/msg/emergency_brake.hpp"
#include "adas_interfaces/msg/obstacle_data.hpp"
#include "adas_interfaces/msg/trajectory.hpp"
#include "adas_interfaces/srv/check_risk.hpp"

class SafetyNode : public rclcpp::Node
{
public:
    SafetyNode();

private:
    // Publishers
    rclcpp::Publisher<adas_interfaces::msg::EmergencyBrake>::SharedPtr pub_emergency_brake_pub_;

    // Subscribers
    rclcpp::Subscription<adas_interfaces::msg::ObstacleData>::SharedPtr sub_obstacle_data_sub_;
    void on_sub_obstacle_data(const adas_interfaces::msg::ObstacleData::SharedPtr msg);
    rclcpp::Subscription<adas_interfaces::msg::Trajectory>::SharedPtr sub_trajectory_sub_;
    void on_sub_trajectory(const adas_interfaces::msg::Trajectory::SharedPtr msg);

    // Service servers
    rclcpp::Service<adas_interfaces::srv::CheckRisk>::SharedPtr server_check_risk_srv_;
    void on_server_check_risk(
        const adas_interfaces::srv::CheckRisk::Request::SharedPtr request,
        adas_interfaces::srv::CheckRisk::Response::SharedPtr response);

    // Parameters
    double emergency_brake_threshold_;
    int64_t reaction_time_ms_;
    bool enable_emergency_override_;
};