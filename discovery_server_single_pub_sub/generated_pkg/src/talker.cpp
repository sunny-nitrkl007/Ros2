#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "talker.hpp"

using namespace std::chrono_literals;

Talker::Talker()

    : Node("talker")

{
    this->declare_parameter<double>("publish_rate", 2.0);
    publish_rate_ = this->get_parameter("publish_rate").as_double();

    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.reliable();
    

    publisher_ = this->create_publisher<std_msgs::msg::String>(
        "chatter", qos);

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
        std::bind(&Talker::publish_message, this));
}

void Talker::publish_message()
{
    auto msg = std_msgs::msg::String();
    //-- begin impl [publish_message] ----------------------------------------
    msg.data = "Hello from talker! count=" + std::to_string(count_++);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
    //-- end impl [publish_message] ----------------------------------------
    publisher_->publish(msg);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Talker>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}