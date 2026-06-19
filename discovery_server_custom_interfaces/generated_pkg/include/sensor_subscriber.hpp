#pragma once


#include "rclcpp/rclcpp.hpp"

#include "custom_interfaces_pkg/msg/sensor.hpp"


class Sensor_subscriber : public rclcpp::Node
{
public:
    Sensor_subscriber();


private:
    void callback(const custom_interfaces_pkg::msg::Sensor::SharedPtr msg);

    rclcpp::Subscription<custom_interfaces_pkg::msg::Sensor>::SharedPtr subscription_;
    std::string log_prefix_;


};
