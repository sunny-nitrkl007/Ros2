#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "std_msgs/msg/string.hpp"

class SiteLogger : public rclcpp::Node
{
public:
    SiteLogger();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr log_group_;

    // Publishers
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr cycle_report_pub_;

    // Subscribers
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr payload_status_sub_;
    void on_payload_status(const geometry_msgs::msg::Vector3::SharedPtr msg);
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr health_telemetry_sub_;
    void on_health_telemetry(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    // Timers
    rclcpp::TimerBase::SharedPtr log_tick_timer_;
    void log_tick();

    // Parameters
    double log_rate_;

    // State
    int    cycle_count_     = 0;
    double total_tonnes_    = 0.0;
    double last_payload_pct_ = 0.0;
    int    fault_count_     = 0;
};