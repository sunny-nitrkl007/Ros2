#include "rsg_v1_service_controlled_sender/receiver.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_service_controlled_sender
{

ReceiverNode::ReceiverNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("receiver", options)
{
  declare_parameters();
  load_parameters();
  setup_parameter_callbacks();
  setup_subscribers();
  setup_watchdogs();
  RCLCPP_INFO(this->get_logger(), "Node 'receiver' started.");
}

ReceiverNode::~ReceiverNode()
{
  RCLCPP_INFO(this->get_logger(), "Node 'receiver' shutting down.");
}

void ReceiverNode::declare_parameters()
{
  this->declare_parameter<bool>("receiving_enabled", true);
}

void ReceiverNode::load_parameters()
{
  this->get_parameter("receiving_enabled", receiving_enabled_);
}

void ReceiverNode::setup_parameter_callbacks()
{
  parameter_callback_handle_ = this->add_on_set_parameters_callback(
    std::bind(&ReceiverNode::on_parameter_update, this, std::placeholders::_1));
}

rcl_interfaces::msg::SetParametersResult ReceiverNode::on_parameter_update(
  const std::vector<rclcpp::Parameter> & parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;

  for (const auto & parameter : parameters) {
    if (parameter.get_name() == "receiving_enabled") {
      receiving_enabled_ = parameter.as_bool();
      RCLCPP_INFO(this->get_logger(), "Updated parameter 'receiving_enabled'.");
    }
  }
  return result;
}

rclcpp::QoS ReceiverNode::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}


void ReceiverNode::setup_subscribers()
{
  heartbeat_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/demo_robot/heartbeat", make_qos_reliable_default(),
    std::bind(&ReceiverNode::on_heartbeat, this, std::placeholders::_1));
}



void ReceiverNode::setup_watchdogs()
{
  heartbeat_sub_watchdog_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(3000),
    std::bind(&ReceiverNode::check_watchdog_heartbeat_sub, this));
}


void ReceiverNode::on_heartbeat(
  const std_msgs::msg::String::SharedPtr msg)
{
  if (!receiving_enabled_) { return; }
  heartbeat_sub_last_msg_time_ = this->now();
  heartbeat_sub_has_msg_ = true;
  if (heartbeat_sub_idle_) {
    heartbeat_sub_idle_ = false;
    RCLCPP_INFO(this->get_logger(), "ACTIVE: messages resumed on /demo_robot/heartbeat.");
  }
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'heartbeat_sub'
  // --------------------------------------------------------------------------
  // Purpose:
  //   This section is intentionally generated as an editable placeholder where
  //   the developer consumes, validates, transforms, or stores received data.
  //
  // Guidance:
  //   - Keep ROS subscription/QoS setup outside this section.
  //   - Add filtering, conversion, state updates, forwarding, or safety checks.
  //   - Avoid long blocking work inside subscriber callbacks.
  //
  // Regeneration note:
  //   In inline_user_sections layout, preserve edits if regenerating in-place.
  // ==========================================================================
  (void)msg;
  // USER LOGIC END
  RCLCPP_INFO(this->get_logger(), "Received message on /demo_robot/heartbeat.");
}

void ReceiverNode::check_watchdog_heartbeat_sub()
{
  if (!heartbeat_sub_has_msg_) {
    if (!heartbeat_sub_idle_) {
      heartbeat_sub_idle_ = true;
      RCLCPP_WARN(this->get_logger(), "IDLE: no first message yet on /demo_robot/heartbeat.");
    }
    return;
  }
  const auto age_ms = (this->now() - heartbeat_sub_last_msg_time_).nanoseconds() / 1000000;
  if (age_ms > 3000) {
    if (!heartbeat_sub_idle_) {
      heartbeat_sub_idle_ = true;
      RCLCPP_WARN(
        this->get_logger(), "IDLE: watchdog timeout on /demo_robot/heartbeat: %ld ms", age_ms);
    }
    // USER LOGIC PLACEHOLDER: add IDLE/recovery behavior here.
  }
}



}  // namespace rsg_v1_service_controlled_sender

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_service_controlled_sender::ReceiverNode>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
