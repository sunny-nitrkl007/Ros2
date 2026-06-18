#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "listener_3.hpp"

Listener_3::Listener_3()

    : Node("listener_3")

{
    this->declare_parameter<std::string>(
        "log_prefix", "[L3] Received:");

    log_prefix_ = this->get_parameter("log_prefix").as_string();

    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.best_effort();
    

    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "chatter", qos,
        std::bind(&Listener_3::callback, this, std::placeholders::_1));
}

void Listener_3::callback(const std_msgs::msg::String::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(), "%s %s",
        log_prefix_.c_str(), msg->data.c_str());
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Listener_3>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}