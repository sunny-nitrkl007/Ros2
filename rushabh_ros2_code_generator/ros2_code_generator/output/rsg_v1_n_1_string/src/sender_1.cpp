#include "rsg_v1_n_1_string/sender_1.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_n_1_string
{

Sender1Node::Sender1Node(const rclcpp::NodeOptions & options)
: rclcpp::Node("sender_1", options)
{
  setup_publishers();
  RCLCPP_INFO(this->get_logger(), "Node 'sender_1' started.");
}

Sender1Node::~Sender1Node()
{
  RCLCPP_INFO(this->get_logger(), "Node 'sender_1' shutting down.");
}


rclcpp::QoS Sender1Node::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}

void Sender1Node::setup_publishers()
{
  main_pub_ = this->create_publisher<std_msgs::msg::String>(
    "/chatter", make_qos_reliable_default());
  const auto main_pub_period_ms = static_cast<int>(1000.0 / 1.0);
  main_pub_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(main_pub_period_ms),
    std::bind(&Sender1Node::publish_main_pub, this));
}





void Sender1Node::publish_main_pub()
{
  auto msg = std_msgs::msg::String();

  // ==========================================================================
  // USER LOGIC START: populate message for publisher 'main_pub'
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
  msg.data = "Hello from sender_1 #" + std::to_string(main_pub_count_);
  // USER LOGIC END

  main_pub_->publish(msg);
  ++main_pub_count_;
  RCLCPP_INFO(this->get_logger(), "Published message on /chatter.");
}




}  // namespace rsg_v1_n_1_string

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_n_1_string::Sender1Node>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
