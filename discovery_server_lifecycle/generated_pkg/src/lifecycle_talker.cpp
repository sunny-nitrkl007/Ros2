#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/string.hpp"
#include "lifecycle_talker.hpp"

using namespace std::chrono_literals;
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

// State machine:
//   unconfigured  -->  on_configure()  -->  inactive
//   inactive      -->  on_activate()   -->  active       (timer starts, publishes)
//   active        -->  on_deactivate() -->  inactive     (timer stops)
//   inactive      -->  on_cleanup()    -->  unconfigured (publisher destroyed)
//   any           -->  on_shutdown()   -->  finalized

Lifecycle_talker::Lifecycle_talker()

    : rclcpp_lifecycle::LifecycleNode("lifecycle_talker")

{
    RCLCPP_INFO(this->get_logger(),
        "Node created. Current state: unconfigured.");
}

CallbackReturn Lifecycle_talker::on_configure(const rclcpp_lifecycle::State &)
{
    this->declare_parameter<double>("publish_rate", 1.0);
    publish_rate_ = this->get_parameter("publish_rate").as_double();

    rclcpp::QoS qos(rclcpp::KeepLast(10));
    
    qos.reliable();
    

    publisher_ = this->create_publisher<std_msgs::msg::String>("chatter", qos);

    RCLCPP_INFO(this->get_logger(),
        "on_configure: publisher created on 'chatter' at %.1f Hz. State: inactive.",
        publish_rate_);
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_talker::on_activate(const rclcpp_lifecycle::State &)
{
    publisher_->on_activate();

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
        std::bind(&Lifecycle_talker::publish_message, this));

    RCLCPP_INFO(this->get_logger(),
        "on_activate: publishing started. State: active.");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_talker::on_deactivate(const rclcpp_lifecycle::State &)
{
    timer_.reset();
    publisher_->on_deactivate();

    RCLCPP_INFO(this->get_logger(),
        "on_deactivate: publishing stopped. State: inactive.");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_talker::on_cleanup(const rclcpp_lifecycle::State &)
{
    publisher_.reset();

    RCLCPP_INFO(this->get_logger(),
        "on_cleanup: publisher destroyed. State: unconfigured.");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_talker::on_shutdown(const rclcpp_lifecycle::State &)
{
    timer_.reset();
    publisher_.reset();

    RCLCPP_INFO(this->get_logger(), "on_shutdown: node finalizing.");
    return CallbackReturn::SUCCESS;
}

void Lifecycle_talker::publish_message()
{
    auto msg = std_msgs::msg::String();
    msg.data = "Hello from lifecycle_talker! count=" + std::to_string(count_++);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
    publisher_->publish(msg);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Lifecycle_talker>());
    rclcpp::shutdown();
    return 0;
}