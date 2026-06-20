#include "rsg_v1_three_node_cross_pubsub_matrix/node_b.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_three_node_cross_pubsub_matrix
{

NodeB::NodeB(const rclcpp::NodeOptions & options)
: rclcpp::Node("node_b", options)
{
  setup_publishers();
  setup_subscribers();
  RCLCPP_INFO(this->get_logger(), "Node 'node_b' started.");
}

NodeB::~NodeB()
{
  RCLCPP_INFO(this->get_logger(), "Node 'node_b' shutting down.");
}


rclcpp::QoS NodeB::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}

void NodeB::setup_publishers()
{
  b_to_a_report_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/matrix_demo/b_to_a/report", make_qos_reliable_default());
  const auto b_to_a_report_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  b_to_a_report_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(b_to_a_report_pub_period_ms),
    std::bind(&NodeB::publish_b_to_a_report_pub, this));
  b_internal_status_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/matrix_demo/b/internal_status", make_qos_reliable_default());
  const auto b_internal_status_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  b_internal_status_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(b_internal_status_pub_period_ms),
    std::bind(&NodeB::publish_b_internal_status_pub, this));
}

void NodeB::setup_subscribers()
{
  a_to_b_event_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/matrix_demo/a_to_b/event", make_qos_reliable_default(),
    std::bind(&NodeB::on_a_event, this, std::placeholders::_1));
  b_internal_status_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/matrix_demo/b/internal_status", make_qos_reliable_default(),
    std::bind(&NodeB::on_internal_status, this, std::placeholders::_1));
}




void NodeB::publish_b_to_a_report_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'b_to_a_report_pub'
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
  msg.data = "Hello from node_b #" + std::to_string(b_to_a_report_pub_count_);
  // USER LOGIC END

  b_to_a_report_pub_->publish(msg);
  ++b_to_a_report_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /matrix_demo/b_to_a/report.");
}
void NodeB::publish_b_internal_status_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'b_internal_status_pub'
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
  msg.data = "Hello from node_b #" + std::to_string(b_internal_status_pub_count_);
  // USER LOGIC END

  b_internal_status_pub_->publish(msg);
  ++b_internal_status_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /matrix_demo/b/internal_status.");
}

void NodeB::on_a_event(
  const std_msgs::msg::String::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'a_to_b_event_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /matrix_demo/a_to_b/event.");
}

void NodeB::on_internal_status(
  const std_msgs::msg::String::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'b_internal_status_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /matrix_demo/b/internal_status.");
}




}  // namespace rsg_v1_three_node_cross_pubsub_matrix

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_three_node_cross_pubsub_matrix::NodeB>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
