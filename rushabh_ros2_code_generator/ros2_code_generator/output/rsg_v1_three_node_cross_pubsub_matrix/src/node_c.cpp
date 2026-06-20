#include "rsg_v1_three_node_cross_pubsub_matrix/node_c.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_three_node_cross_pubsub_matrix
{

NodeC::NodeC(const rclcpp::NodeOptions & options)
: rclcpp::Node("node_c", options)
{
  setup_publishers();
  setup_subscribers();
  RCLCPP_INFO(this->get_logger(), "Node 'node_c' started.");
}

NodeC::~NodeC()
{
  RCLCPP_INFO(this->get_logger(), "Node 'node_c' shutting down.");
}


rclcpp::QoS NodeC::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}

void NodeC::setup_publishers()
{
  c_to_a_feedback_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/matrix_demo/c_to_a/feedback", make_qos_reliable_default());
  const auto c_to_a_feedback_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  c_to_a_feedback_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(c_to_a_feedback_pub_period_ms),
    std::bind(&NodeC::publish_c_to_a_feedback_pub, this));
}

void NodeC::setup_subscribers()
{
  a_to_c_status_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/matrix_demo/a_to_c/status", make_qos_reliable_default(),
    std::bind(&NodeC::on_a_status, this, std::placeholders::_1));
  a_to_c_command_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/matrix_demo/a_to_c/command", make_qos_reliable_default(),
    std::bind(&NodeC::on_a_command, this, std::placeholders::_1));
}




void NodeC::publish_c_to_a_feedback_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'c_to_a_feedback_pub'
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
  msg.data = "Hello from node_c #" + std::to_string(c_to_a_feedback_pub_count_);
  // USER LOGIC END

  c_to_a_feedback_pub_->publish(msg);
  ++c_to_a_feedback_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /matrix_demo/c_to_a/feedback.");
}

void NodeC::on_a_status(
  const std_msgs::msg::String::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'a_to_c_status_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /matrix_demo/a_to_c/status.");
}

void NodeC::on_a_command(
  const std_msgs::msg::String::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'a_to_c_command_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /matrix_demo/a_to_c/command.");
}




}  // namespace rsg_v1_three_node_cross_pubsub_matrix

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_three_node_cross_pubsub_matrix::NodeC>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
