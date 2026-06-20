#include "rsg_v1_n_n_msg/receiver_2.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_n_n_msg
{

Receiver2Node::Receiver2Node(const rclcpp::NodeOptions & options)
: rclcpp::Node("receiver_2", options)
{
  setup_subscribers();
  RCLCPP_INFO(this->get_logger(), "Node 'receiver_2' started.");
}

Receiver2Node::~Receiver2Node()
{
  RCLCPP_INFO(this->get_logger(), "Node 'receiver_2' shutting down.");
}


rclcpp::QoS Receiver2Node::make_qos_reliable_default() const
{
  rclcpp::QoS qos(rclcpp::KeepLast(10));
  qos.reliable();
  qos.durability_volatile();
  return qos;
}


void Receiver2Node::setup_subscribers()
{
  main_sub_ = this->create_subscription<rsg_v1_n_n_msg::msg::MachineStatus>(
    "/machine_status", make_qos_reliable_default(),
    std::bind(&Receiver2Node::on_message, this, std::placeholders::_1));
}





void Receiver2Node::on_message(
  const rsg_v1_n_n_msg::msg::MachineStatus::SharedPtr msg)
{
  // ==========================================================================
  // USER LOGIC START: process message for subscriber 'main_sub'
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
  RCLCPP_INFO(this->get_logger(), "Received message on /machine_status.");
}




}  // namespace rsg_v1_n_n_msg

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_n_n_msg::Receiver2Node>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
