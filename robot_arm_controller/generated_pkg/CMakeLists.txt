cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(trajectory_msgs REQUIRED)

add_executable(arm_controller src/arm_controller.cpp)
target_include_directories(arm_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(arm_controller PUBLIC cxx_std_17)
ament_target_dependencies(arm_controller
  rclcpp
  sensor_msgs
  std_msgs
  std_srvs
  trajectory_msgs
)

add_executable(collision_detector src/collision_detector.cpp)
target_include_directories(collision_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(collision_detector PUBLIC cxx_std_17)
ament_target_dependencies(collision_detector
  rclcpp
  sensor_msgs
  std_msgs
  std_srvs
  trajectory_msgs
)

install(TARGETS
  arm_controller
  collision_detector
  DESTINATION lib/${PROJECT_NAME})

ament_package()