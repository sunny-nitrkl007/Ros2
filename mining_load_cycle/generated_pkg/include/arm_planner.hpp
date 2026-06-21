#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/trigger.hpp"

class ArmPlanner : public rclcpp::Node
{
public:
    ArmPlanner();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr arm_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr dig_command_pub_;

    // Subscribers
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr proximity_alert_sub_;
    void on_proximity_alert(const std_msgs::msg::Bool::SharedPtr msg);

    // Service clients
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr request_load_client_;

    // Timers
    rclcpp::TimerBase::SharedPtr dig_cycle_timer_;
    void dig_cycle();

    // Parameters
    double dig_rate_;
    double dig_depth_;
    double swing_speed_;

    // State — 0=IDLE 1=DIGGING 2=RAISING 3=DUMPING
    int    arm_state_     = 0;
    bool   truck_ready_   = false;
    double current_angle_ = -90.0;
    int    dig_tick_      = 0;
};