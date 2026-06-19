#pragma once

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"


class Robot1_talker : public rclcpp::Node
{
public:
    Robot1_talker();


private:
    void publish_message();

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    double publish_rate_;
    size_t count_ = 0;


};