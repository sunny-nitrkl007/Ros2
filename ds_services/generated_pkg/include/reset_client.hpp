#pragma once

#include "rclcpp/rclcpp.hpp"

#include "std_srvs/srv/trigger.hpp"


class Reset_client : public rclcpp::Node
{
public:
    Reset_client();


private:
    void send_request();

    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr timer_;


};