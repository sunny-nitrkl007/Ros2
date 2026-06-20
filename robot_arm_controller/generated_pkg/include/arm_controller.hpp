#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/set_bool.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "trajectory_msgs/msg/joint_trajectory.hpp"

class ArmController : public rclcpp::Node
{
public:
    ArmController();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr control_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_states_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr arm_status_pub_;

    // Subscribers
    rclcpp::Subscription<trajectory_msgs::msg::JointTrajectory>::SharedPtr joint_commands_sub_;
    void on_joint_commands(const trajectory_msgs::msg::JointTrajectory::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr set_mode_srv_;
    void on_set_mode(
        const std_srvs::srv::SetBool::Request::SharedPtr request,
        std_srvs::srv::SetBool::Response::SharedPtr response);

    // Service clients
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr check_collision_client_;

    // Timers
    rclcpp::TimerBase::SharedPtr control_loop_timer_;
    void control_loop();

    // Parameters
    std::vector<std::string> joint_names_;
    double max_velocity_;
    double control_rate_;
    std::string mode_;
};