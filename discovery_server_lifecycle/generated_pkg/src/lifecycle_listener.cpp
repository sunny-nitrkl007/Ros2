#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/string.hpp"
#include "lifecycle_listener.hpp"

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

// State machine:
//   unconfigured  -->  on_configure()  -->  inactive  (subscription created, callbacks fire but ignored)
//   inactive      -->  on_activate()   -->  active    (active_ = true, messages now logged)
//   active        -->  on_deactivate() -->  inactive  (active_ = false, messages silently dropped)
//   inactive      -->  on_cleanup()    -->  unconfigured (subscription destroyed)
//   any           -->  on_shutdown()   -->  finalized

Lifecycle_listener::Lifecycle_listener()

    : rclcpp_lifecycle::LifecycleNode("lifecycle_listener")

{
    RCLCPP_INFO(this->get_logger(),
        "Node created. Current state: unconfigured.");
}

CallbackReturn Lifecycle_listener::on_configure(const rclcpp_lifecycle::State &)
{
    this->declare_parameter<std::string>(
        "log_prefix", "[lifecycle] Received:");
    log_prefix_ = this->get_parameter("log_prefix").as_string();

    rclcpp::QoS qos(rclcpp::KeepLast(10));
    
    qos.best_effort();
    

    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "chatter", qos,
        std::bind(&Lifecycle_listener::callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(),
        "on_configure: subscribed to 'chatter'. State: inactive (messages ignored until active).");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_listener::on_activate(const rclcpp_lifecycle::State &)
{
    active_ = true;
    RCLCPP_INFO(this->get_logger(),
        "on_activate: now processing messages. State: active.");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_listener::on_deactivate(const rclcpp_lifecycle::State &)
{
    active_ = false;
    RCLCPP_INFO(this->get_logger(),
        "on_deactivate: messages arriving but ignored. State: inactive.");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_listener::on_cleanup(const rclcpp_lifecycle::State &)
{
    subscription_.reset();
    RCLCPP_INFO(this->get_logger(),
        "on_cleanup: subscription destroyed. State: unconfigured.");
    return CallbackReturn::SUCCESS;
}

CallbackReturn Lifecycle_listener::on_shutdown(const rclcpp_lifecycle::State &)
{
    subscription_.reset();
    RCLCPP_INFO(this->get_logger(), "on_shutdown: node finalizing.");
    return CallbackReturn::SUCCESS;
}

void Lifecycle_listener::callback(const std_msgs::msg::String::SharedPtr msg)
{
    if (!active_) {
        return;
    }
    RCLCPP_INFO(this->get_logger(), "%s %s",
        log_prefix_.c_str(), msg->data.c_str());
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Lifecycle_listener>());
    rclcpp::shutdown();
    return 0;
}