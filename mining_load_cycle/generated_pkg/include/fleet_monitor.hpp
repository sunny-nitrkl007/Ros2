#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "std_srvs/srv/trigger.hpp"

class FleetMonitor : public rclcpp::Node
{
public:
    FleetMonitor();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr monitor_group_;

    // Publishers
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr health_telemetry_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr bucket_state_sub_;
    void on_bucket_state(const geometry_msgs::msg::Vector3::SharedPtr msg);
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr truck_pose_sub_;
    void on_truck_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr payload_status_sub_;
    void on_payload_status(const geometry_msgs::msg::Vector3::SharedPtr msg);

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr emergency_stop_srv_;
    void on_emergency_stop(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Timers
    rclcpp::TimerBase::SharedPtr monitor_tick_timer_;
    void monitor_tick();

    // Parameters
    double monitor_rate_;

    // State
    double last_bucket_load_  = 0.0;
    double last_truck_x_      = 0.0;
    double last_truck_y_      = 0.0;
    double last_payload_pct_  = 0.0;
    bool   emergency_active_  = false;
};