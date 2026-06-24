cmake_minimum_required(VERSION 3.8)
project(perception_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(adas_interfaces REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(perception1_node src/perception1_node.cpp)
target_include_directories(perception1_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(perception1_node PUBLIC cxx_std_17)
ament_target_dependencies(perception1_node
  adas_interfaces
  rclcpp
)

add_executable(perception2_node src/perception2_node.cpp)
target_include_directories(perception2_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(perception2_node PUBLIC cxx_std_17)
ament_target_dependencies(perception2_node
  adas_interfaces
  rclcpp
)

install(TARGETS
  perception1_node
  perception2_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()