#ifndef ROS2_BRIDGE_APP_NATIVE_SWITCH_INPUT_ROS_ADAPTER_HPP_
#define ROS2_BRIDGE_APP_NATIVE_SWITCH_INPUT_ROS_ADAPTER_HPP_

#include <interfaces/SwitchInputScs/InterfaceTypes.h>
#include "ros2_bridge_app/msg/switch_input_scs.hpp"

class NativeSwitchInputRosAdapter
{
public:
  explicit NativeSwitchInputRosAdapter(SwitchInputScsInput * native_input);
  void get(ros2_bridge_app::msg::SwitchInputScs & ros_message);

private:
  SwitchInputScsInput * native_input_;
};

#endif
