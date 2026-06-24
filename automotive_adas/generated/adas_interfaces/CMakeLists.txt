cmake_minimum_required(VERSION 3.8)
project(adas_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/ObstacleData.msg"
  "msg/Trajectory.msg"
  "msg/EmergencyBrake.msg"
  "srv/GetLaneInfo.srv"
  "srv/CheckRisk.srv"
  "srv/RequestPathUpdate.srv"
)

ament_package()