#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "robot2_talker.hpp"

using namespace std::chrono_literals;

Robot2_talker::Robot2_talker()

    : Node("robot2_talker", "/robot2")

{
    this->declare_parameter<double>("publish_rate", 1.0);
    publish_rate_ = this->get_parameter("publish_rate").as_double();

    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.reliable();
    

    publisher_ = this->create_publisher<std_msgs::msg::String>(
        "chatter", qos);

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
        std::bind(&Robot2_talker::publish_message, this));
}

void Robot2_talker::publish_message()
{
    auto msg = std_msgs::msg::String();
    msg.data = "Hello from robot2_talker! count=" + std::to_string(count_++);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
    publisher_->publish(msg);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Robot2_talker>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}