#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/trigger.hpp"

class CollisionDetector : public rclcpp::Node
{
public:
    CollisionDetector();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr safety_group_;

    // Publishers
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr collision_alert_pub_;

    // Subscribers
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_states_sub_;
    void on_joint_states(const sensor_msgs::msg::JointState::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr check_collision_srv_;
    void on_check_collision(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Timers
    rclcpp::TimerBase::SharedPtr safety_check_timer_;
    void safety_check();

    // Parameters
    double safety_margin_;
    double check_rate_;
    bool alert_on_proximity_;

    // Runtime state
    std::vector<double> latest_positions_;
    bool collision_detected_ = false;
};