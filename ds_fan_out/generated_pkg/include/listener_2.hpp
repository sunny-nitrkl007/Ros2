#pragma once

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"


class Listener_2 : public rclcpp::Node
{
public:
    Listener_2();


private:
    void callback(const std_msgs::msg::String::SharedPtr msg);

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    std::string log_prefix_;


};