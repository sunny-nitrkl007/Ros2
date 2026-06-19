cmake_minimum_required(VERSION 3.8)
project(custom_interfaces_pkg)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Sensor.msg"
  "srv/SensorQuery.srv"
)

ament_package()
