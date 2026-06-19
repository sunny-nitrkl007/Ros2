#pragma once


#include "rclcpp/rclcpp.hpp"

#include "custom_interfaces_pkg/msg/sensor.hpp"


class Sensor_publisher : public rclcpp::Node
{
public:
    Sensor_publisher();


private:
    void publish_message();

    rclcpp::Publisher<custom_interfaces_pkg::msg::Sensor>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::string sensor_id_;
    double publish_rate_;
    size_t count_ = 0;


};
