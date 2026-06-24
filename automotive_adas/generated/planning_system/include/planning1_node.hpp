#pragma once

#include "rclcpp/rclcpp.hpp"
#include "adas_interfaces/msg/emergency_brake.hpp"
#include "adas_interfaces/msg/obstacle_data.hpp"
#include "adas_interfaces/msg/trajectory.hpp"

class Planning1Node : public rclcpp::Node
{
public:
    Planning1Node();

private:
    // Publishers
    rclcpp::Publisher<adas_interfaces::msg::Trajectory>::SharedPtr pub_trajectory_pub_;

    // Subscribers
    rclcpp::Subscription<adas_interfaces::msg::ObstacleData>::SharedPtr sub_obstacle_data_sub_;
    void on_sub_obstacle_data(const adas_interfaces::msg::ObstacleData::SharedPtr msg);
    rclcpp::Subscription<adas_interfaces::msg::EmergencyBrake>::SharedPtr sub_emergency_brake_sub_;
    void on_sub_emergency_brake(const adas_interfaces::msg::EmergencyBrake::SharedPtr msg);

    // Parameters
    double planning_rate_hz_;
    double max_speed_mps_;
    double safe_follow_distance_m_;
};