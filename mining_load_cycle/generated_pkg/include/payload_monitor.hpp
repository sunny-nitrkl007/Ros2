#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "std_srvs/srv/trigger.hpp"

class PayloadMonitor : public rclcpp::Node
{
public:
    PayloadMonitor();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr payload_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr payload_status_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr bucket_state_sub_;
    void on_bucket_state(const geometry_msgs::msg::Vector3::SharedPtr msg);

    // Service clients
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr check_payload_client_;

    // Parameters
    double target_payload_kg_;
    double check_interval_;

    // State
    double total_payload_kg_  = 0.0;
    double last_load_pct_     = 0.0;
    bool   payload_full_      = false;
};