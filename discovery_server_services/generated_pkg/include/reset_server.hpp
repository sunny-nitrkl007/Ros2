#pragma once


#include "rclcpp/rclcpp.hpp"

#include "std_srvs/srv/trigger.hpp"


class Reset_server : public rclcpp::Node
{
public:
    Reset_server();


private:
    void handle_request(
        const std_srvs::srv::Trigger::Request::SharedPtr request,
        std_srvs::srv::Trigger::Response::SharedPtr response);

    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service_;


};
