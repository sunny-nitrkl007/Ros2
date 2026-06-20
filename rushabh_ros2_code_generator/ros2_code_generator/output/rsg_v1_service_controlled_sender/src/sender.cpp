#include "rsg_v1_service_controlled_sender/sender.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_service_controlled_sender
{

SenderNode::SenderNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("sender", options)
{
  declare_parameters();
  load_parameters();
  setup_parameter_callbacks();
  setup_publishers();
  setup_services();
  RCLCPP_INFO(this->get_logger(), "Node 'sender' started.");
}

SenderNode::~SenderNode()
{
  RCLCPP_INFO(this->get_logger(), "Node 'sender' shutting down.");
}

void SenderNode::declare_parameters()
{
  this->declare_parameter<double>("publish_rate_hz", 1.0);
  this->declare_parameter<bool>("enabled", true);
  this->declare_parameter<bool>("sending_enabled", true);
}

void SenderNode::load_parameters()
{
  this->get_parameter("publish_rate_hz", publish_rate_hz_);
  this->get_parameter("enabled", enabled_);
  this->get_parameter("sending_enabled", sending_enabled_);
}

void SenderNode::setup_parameter_callbacks()
{
  parameter_callback_handle_ = this->add_on_set_parameters_callback(
    std::bind(&SenderNode::on_parameter_update, this, std::placeholders::_1));
}

rcl_interfaces::msg::SetParametersResult SenderNode::on_parameter_update(
  const std::vector<rclcpp::Parameter> & parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;

  for (const auto & parameter : parameters) {
    if (parameter.get_name() == "publish_rate_hz") {
      const auto value = parameter.as_double();
      if (value < 0.1) { result.successful = false; result.reason = "publish_rate_hz below min"; return result; }
      if (value > 20.0) { result.successful = false; result.reason = "publish_rate_hz above max"; return result; }
      publish_rate_hz_ = value;
      RCLCPP_INFO(this->get_logger(), "Updated parameter 'publish_rate_hz'.");
    }
    if (parameter.get_name() == "enabled") {
      enabled_ = parameter.as_bool();
      RCLCPP_INFO(this->get_logger(), "Updated parameter 'enabled'.");
    }
    if (parameter.get_name() == "sending_enabled") {
      sending_enabled_ = parameter.as_bool();
      RCLCPP_INFO(this->get_logger(), "Updated parameter 'sending_enabled'.");
    }
  }
  return result;
}

rclcpp::QoS SenderNode::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}

void SenderNode::setup_publishers()
{
  heartbeat_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/demo_robot/heartbeat", make_qos_reliable_default());
  const auto heartbeat_pub_period_ms = static_cast<int>(1000.0 / publish_rate_hz_);
  heartbeat_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(heartbeat_pub_period_ms),
    std::bind(&SenderNode::publish_heartbeat_pub, this));
}


void SenderNode::setup_services()
{
  set_sending_service_ = this->create_service<rsg_v1_service_controlled_sender::srv::SetSending>(
    "/demo_robot/set_sending",
    std::bind(&SenderNode::on_set_sending_request, this, std::placeholders::_1, std::placeholders::_2));
}



void SenderNode::publish_heartbeat_pub()
{
  if (!enabled_) { return; }
  if (!sending_enabled_) { return; }
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'heartbeat_pub'
  // --------------------------------------------------------------------------
  // Purpose:
  //   This section is intentionally generated as an editable placeholder where
  //   the developer fills outgoing message fields before publish().
  //
  // Guidance:
  //   - Keep ROS publisher/timer/QoS infrastructure outside this section.
  //   - Add application logic here: counters, state, sensor values, status text,
  //     command values, timestamps, or data-model conversion.
  //   - For custom messages, assign each required field explicitly.
  //
  // Regeneration note:
  //   In inline_user_sections layout, preserve edits if regenerating in-place.
  // ==========================================================================
  msg.data = "Hello from sender #" + std::to_string(heartbeat_pub_count_);
  // USER LOGIC END

  heartbeat_pub_->publish(msg);
  ++heartbeat_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /demo_robot/heartbeat.");
}


void SenderNode::on_set_sending_request(
  const std::shared_ptr<rsg_v1_service_controlled_sender::srv::SetSending::Request> request,
  std::shared_ptr<rsg_v1_service_controlled_sender::srv::SetSending::Response> response)
{
  // ==========================================================================
  // USER LOGIC START: fill service response for 'set_sending_service'
  // --------------------------------------------------------------------------
  // Purpose:
  //   Inspect the request and fill the response. If this service has a
  //   control block in grammar, generated logic updates the target node
  //   parameter/state from the request field.
  // ==========================================================================
  // RSG V1 service-control generated logic.
  // request->enabled updates 'sending_enabled_'.
  sending_enabled_ = request->enabled;
  response->success = true;
  response->message = request->enabled
    ? "sending_enabled enabled."
    : "sending_enabled disabled.";
  // USER LOGIC END
}


}  // namespace rsg_v1_service_controlled_sender

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_service_controlled_sender::SenderNode>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
