#pragma once

#include "rclcpp/rclcpp.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "std_srvs/srv/trigger.hpp"

class TruckHealthMonitor : public rclcpp::Node
{
public:
    TruckHealthMonitor();

private:
    // Callback groups
    rclcpp::CallbackGroup::SharedPtr health_group_;

    // Publishers
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr health_report_pub_;

    // Service servers
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr system_status_srv_;
    void on_system_status(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    // Timers
    rclcpp::TimerBase::SharedPtr health_tick_timer_;
    void health_tick();

    // Parameters
    double engine_temp_max_;
    double hydraulic_pressure_min_;
    double report_rate_;

    // State
    uint64_t tick_            = 0;
    double   engine_temp_     = 72.0;
    double   hydraulic_press_ = 185.0;
    double   payload_kg_      = 0.0;
};