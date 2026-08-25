#ifndef ROS2_BRIDGE_APP_ROS2_BRIDGE_APP_HPP_
#define ROS2_BRIDGE_APP_ROS2_BRIDGE_APP_HPP_

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include <ais/task/Task.h>
#include <interfaces/SwitchInputScs/InterfaceTypes.h>
#include <rclcpp/rclcpp.hpp>

#include "ros2_bridge_app/NativeSwitchInputRosAdapter.hpp"
#include "ros2_bridge_app/msg/switch_input_scs.hpp"

class Ros2BridgeApp : public task::Task
{
public:
  explicit Ros2BridgeApp(const std::string & taskName);
  ~Ros2BridgeApp() override;

  bool initialize() override;
  bool executive() override;
  void cleanup() override;

private:
  void bridgeThreadFunction();

  SwitchInputScsInput * lpsSaSwitchInput_;
  std::unique_ptr<NativeSwitchInputRosAdapter> nativeAdapter_;
  NativeSwitchInputRosAdapter * LpsSaSwitchInput;

  rclcpp::Node::SharedPtr rosNode_;
  rclcpp::Publisher<ros2_bridge_app::msg::SwitchInputScs>::SharedPtr publisher_;

  std::atomic<bool> running_;
  std::thread bridgeThread_;
};

#endif
