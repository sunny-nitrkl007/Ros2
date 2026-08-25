#include "ros2_bridge_app/NativeSwitchInputRosAdapter.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

NativeSwitchInputRosAdapter::NativeSwitchInputRosAdapter(
  SwitchInputScsInput * native_input)
: native_input_(native_input)
{
  if (native_input_ == nullptr) {
    throw std::invalid_argument("native SwitchInputScs input must not be null");
  }
}

void NativeSwitchInputRosAdapter::get(
  ros2_bridge_app::msg::SwitchInputScs & ros_message)
{
  SwitchInputScsStorage native_message{};
  native_input_->get(native_message);

  for (std::size_t i = 0; i < ros_message.stg_values.size(); ++i) {
    ros_message.stg_values[i].value =
      static_cast<std::uint8_t>(native_message.stg_values[i].value);
  }
}
