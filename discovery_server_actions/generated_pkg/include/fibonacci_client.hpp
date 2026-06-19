#pragma once


#include "rclcpp/rclcpp.hpp"

#include "rclcpp_action/rclcpp_action.hpp"
#include "example_interfaces/action/fibonacci.hpp"
#include <thread>


class Fibonacci_client : public rclcpp::Node
{
public:
    Fibonacci_client();


private:
    void send_goal();

    rclcpp_action::Client<example_interfaces::action::Fibonacci>::SharedPtr action_client_;
    rclcpp::TimerBase::SharedPtr timer_;


};
