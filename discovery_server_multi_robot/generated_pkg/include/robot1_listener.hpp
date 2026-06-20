#pragma once


#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"


class Robot1_listener : public rclcpp::Node
{
public:
    Robot1_listener();


private:
    void callback(const std_msgs::msg::String::SharedPtr msg);

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    std::string log_prefix_;


};
