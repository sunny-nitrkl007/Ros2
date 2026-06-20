#pragma once


#include "rclcpp/rclcpp.hpp"

#include "custom_interfaces_pkg/srv/sensor_query.hpp"


class Sensor_query_client : public rclcpp::Node
{
public:
    Sensor_query_client();


private:
    void send_request();

    rclcpp::Client<custom_interfaces_pkg::srv::SensorQuery>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::string target_sensor_id_;
    double query_interval_s_;


};
