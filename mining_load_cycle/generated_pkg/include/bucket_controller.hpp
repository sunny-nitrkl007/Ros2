#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "std_srvs/srv/trigger.hpp"

class BucketController : public rclcpp::Node
{
public:
    BucketController();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr bucket_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr bucket_state_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr dig_command_sub_;
    void on_dig_command(const geometry_msgs::msg::Vector3::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr check_payload_srv_;
    void on_check_payload(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Parameters
    double max_load_kg_;
    double fill_rate_;
    double dump_threshold_;

    // State
    double current_angle_   = -90.0;
    double target_angle_    = -90.0;
    double current_load_kg_ = 0.0;
    bool   is_digging_      = false;
};