#pragma once

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"


class Robot2_listener : public rclcpp::Node
{
public:
    Robot2_listener();


private:
    void callback(const std_msgs::msg::String::SharedPtr msg);

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    std::string log_prefix_;


};