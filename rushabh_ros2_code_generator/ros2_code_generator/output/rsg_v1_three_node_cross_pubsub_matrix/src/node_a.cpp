#include "rsg_v1_three_node_cross_pubsub_matrix/node_a.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_three_node_cross_pubsub_matrix
{

NodeA::NodeA(const rclcpp::NodeOptions & options)
: rclcpp::Node("node_a", options)
{
  setup_publishers();
  setup_subscribers();
  RCLCPP_INFO(this->get_logger(), "Node 'node_a' started.");
}

NodeA::~NodeA()
{
  RCLCPP_INFO(this->get_logger(), "Node 'node_a' shutting down.");
}


rclcpp::QoS NodeA::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}

void NodeA::setup_publishers()
{
  a_to_c_status_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/matrix_demo/a_to_c/status", make_qos_reliable_default());
  const auto a_to_c_status_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  a_to_c_status_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(a_to_c_status_pub_period_ms),
    std::bind(&NodeA::publish_a_to_c_status_pub, this));
  a_to_c_command_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/matrix_demo/a_to_c/command", make_qos_reliable_default());
  const auto a_to_c_command_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  a_to_c_command_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(a_to_c_command_pub_period_ms),
    std::bind(&NodeA::publish_a_to_c_command_pub, this));
  a_to_b_event_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/matrix_demo/a_to_b/event", make_qos_reliable_default());
  const auto a_to_b_event_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  a_to_b_event_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(a_to_b_event_pub_period_ms),
    std::bind(&NodeA::publish_a_to_b_event_pub, this));
}

void NodeA::setup_subscribers()
{
  c_to_a_feedback_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/matrix_demo/c_to_a/feedback", make_qos_reliable_default(),
    std::bind(&NodeA::on_c_feedback, this, std::placeholders::_1));
  b_to_a_report_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/matrix_demo/b_to_a/report", make_qos_reliable_default(),
    std::bind(&NodeA::on_b_report, this, std::placeholders::_1));
}




void NodeA::publish_a_to_c_status_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'a_to_c_status_pub'
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
  msg.data = "Hello from node_a #" + std::to_string(a_to_c_status_pub_count_);
  // USER LOGIC END

  a_to_c_status_pub_->publish(msg);
  ++a_to_c_status_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /matrix_demo/a_to_c/status.");
}
void NodeA::publish_a_to_c_command_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'a_to_c_command_pub'
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
  msg.data = "Hello from node_a #" + std::to_string(a_to_c_command_pub_count_);
  // USER LOGIC END

  a_to_c_command_pub_->publish(msg);
  ++a_to_c_command_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /matrix_demo/a_to_c/command.");
}
void NodeA::publish_a_to_b_event_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'a_to_b_event_pub'
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
  msg.data = "Hello from node_a #" + std::to_string(a_to_b_event_pub_count_);
  // USER LOGIC END

  a_to_b_event_pub_->publish(msg);
  ++a_to_b_event_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /matrix_demo/a_to_b/event.");
}

void NodeA::on_c_feedback(
  const std_msgs::msg::String::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'c_to_a_feedback_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /matrix_demo/c_to_a/feedback.");
}

void NodeA::on_b_report(
  const std_msgs::msg::String::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'b_to_a_report_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /matrix_demo/b_to_a/report.");
}




}  // namespace rsg_v1_three_node_cross_pubsub_matrix

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_three_node_cross_pubsub_matrix::NodeA>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
