#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "robot2_listener.hpp"

Robot2_listener::Robot2_listener()

    : Node("robot2_listener", "/robot2")

{
    this->declare_parameter<std::string>(
        "log_prefix", "[robot2] Received:");

    log_prefix_ = this->get_parameter("log_prefix").as_string();

    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.best_effort();
    

    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "chatter", qos,
        std::bind(&Robot2_listener::callback, this, std::placeholders::_1));
}

void Robot2_listener::callback(const std_msgs::msg::String::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(), "%s %s",
        log_prefix_.c_str(), msg->data.c_str());
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Robot2_listener>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}