#pragma once


#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"


class Listener_3 : public rclcpp::Node
{
public:
    Listener_3();


private:
    void callback(const std_msgs::msg::String::SharedPtr msg);

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    std::string log_prefix_;


};
