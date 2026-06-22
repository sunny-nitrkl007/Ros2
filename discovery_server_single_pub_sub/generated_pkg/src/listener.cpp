#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "listener.hpp"

Listener::Listener()

    : Node("listener")

{
    this->declare_parameter<std::string>(
        "log_prefix", "Received:");

    log_prefix_ = this->get_parameter("log_prefix").as_string();

    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.best_effort();
    

    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "chatter", qos,
        std::bind(&Listener::callback, this, std::placeholders::_1));
}

void Listener::callback(const std_msgs::msg::String::SharedPtr msg)
{
    //-- begin impl [callback] ----------------------------------------
    RCLCPP_INFO(this->get_logger(), "%s %s",
        log_prefix_.c_str(), msg->data.c_str());
    //-- end impl [callback] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Listener>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}