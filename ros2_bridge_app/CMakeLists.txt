cmake_minimum_required(VERSION 3.16)
project(ros2_bridge_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/STG.msg"
  "msg/SwitchInputScs.msg"
)

add_library(${PROJECT_NAME} SHARED
  src/Ros2BridgeApp.cpp
  src/NativeSwitchInputRosAdapter.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(${PROJECT_NAME} rclcpp)
rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(${PROJECT_NAME} "${cpp_typesupport_target}")

# Add the platform libraries that provide AIS Task, InterfaceDb, and SwitchInputScs.
# target_link_libraries(${PROJECT_NAME} PRIVATE <ais_task_lib> <interface_db_lib> <switch_input_scs_lib>)

install(DIRECTORY include/ DESTINATION include)
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
