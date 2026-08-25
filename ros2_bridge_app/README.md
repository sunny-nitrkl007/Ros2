# ROS2 Bridge App

Hybrid bridge that fetches the registered native `SwitchInputScsInput`, reads
STG data, converts it to a ROS 2 message, and publishes it on
`/switch_input_scs`.

## Initialization

```cpp
lpsSaSwitchInput_ =
  dynamic_cast<SwitchInputScsInput *>(
    InterfaceDb::fetch("SwitchInputScsInput"));
```

The package also includes `config/ROS2BridgeApp.rb`, which registers
`SwitchInputScsInput` and sets the executable name to `ROS2BridgeApp`.

## Preserved bridge statements

```cpp
LpsSaSwitchInput->get(STG_Input);
publisher_->publish(STG_Input);
```

## Integration notes

1. Confirm the platform header path for `InterfaceDb`. The current source uses
   `<ais/interface/InterfaceDb.h>`; adjust only the include if your SDK differs.
2. Add the platform libraries providing AIS Task, InterfaceDb, and
   SwitchInputScs to `CMakeLists.txt`.
3. If the native storage field names differ, update only
   `NativeSwitchInputRosAdapter.cpp`.

## Build

```bash
cd ~/ros2_ws
colcon build --packages-select ros2_bridge_app
source install/setup.bash
```

## Verify

```bash
ros2 topic echo /switch_input_scs
ros2 topic hz /switch_input_scs
```
