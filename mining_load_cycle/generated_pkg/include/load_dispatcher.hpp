#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"

class LoadDispatcher : public rclcpp::Node
{
public:
    LoadDispatcher();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr dispatch_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;

    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr waypoint_command_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr load_assignment_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr truck_pose_sub_;
    void on_truck_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr payload_status_sub_;
    void on_payload_status(const geometry_msgs::msg::Vector3::SharedPtr msg);
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr proximity_alert_sub_;
    void on_proximity_alert(const std_msgs::msg::Bool::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr request_load_srv_;
    void on_request_load(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Service clients
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr emergency_stop_client_;

    // Timers
    rclcpp::TimerBase::SharedPtr cycle_tick_timer_;
    void cycle_tick();

    // Parameters
    double cycle_rate_;
    double dig_zone_x_;
    double dig_zone_y_;
    double dump_zone_x_;
    double dump_zone_y_;

    // State — 0=POSITIONING 1=WAITING 2=LOADING 3=DISPATCHING
    int    cycle_state_     = 0;
    int    cycles_complete_ = 0;
    double truck_x_         = 0.0;
    double truck_y_         = 0.0;
    bool   truck_at_zone_   = false;
    bool   payload_full_    = false;
    bool   load_requested_  = false;
};