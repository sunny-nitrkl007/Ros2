#pragma once

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"


class Listener_1 : public rclcpp::Node
{
public:
    Listener_1();


private:
    void callback(const std_msgs::msg::String::SharedPtr msg);

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    std::string log_prefix_;


};