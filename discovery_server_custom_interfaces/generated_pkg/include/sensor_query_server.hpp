#pragma once


#include "rclcpp/rclcpp.hpp"

#include "custom_interfaces_pkg/srv/sensor_query.hpp"


class Sensor_query_server : public rclcpp::Node
{
public:
    Sensor_query_server();


private:
    void handle_request(
        const custom_interfaces_pkg::srv::SensorQuery::Request::SharedPtr request,
        custom_interfaces_pkg::srv::SensorQuery::Response::SharedPtr response);

    rclcpp::Service<custom_interfaces_pkg::srv::SensorQuery>::SharedPtr service_;


};
