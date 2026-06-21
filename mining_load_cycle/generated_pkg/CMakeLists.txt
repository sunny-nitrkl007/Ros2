cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

add_executable(arm_planner src/arm_planner.cpp)
target_include_directories(arm_planner PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(arm_planner PUBLIC cxx_std_17)
ament_target_dependencies(arm_planner
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(bucket_controller src/bucket_controller.cpp)
target_include_directories(bucket_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(bucket_controller PUBLIC cxx_std_17)
ament_target_dependencies(bucket_controller
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(proximity_detector src/proximity_detector.cpp)
target_include_directories(proximity_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(proximity_detector PUBLIC cxx_std_17)
ament_target_dependencies(proximity_detector
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(truck_navigator src/truck_navigator.cpp)
target_include_directories(truck_navigator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(truck_navigator PUBLIC cxx_std_17)
ament_target_dependencies(truck_navigator
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(payload_monitor src/payload_monitor.cpp)
target_include_directories(payload_monitor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(payload_monitor PUBLIC cxx_std_17)
ament_target_dependencies(payload_monitor
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(load_dispatcher src/load_dispatcher.cpp)
target_include_directories(load_dispatcher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(load_dispatcher PUBLIC cxx_std_17)
ament_target_dependencies(load_dispatcher
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(fleet_monitor src/fleet_monitor.cpp)
target_include_directories(fleet_monitor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(fleet_monitor PUBLIC cxx_std_17)
ament_target_dependencies(fleet_monitor
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

add_executable(site_logger src/site_logger.cpp)
target_include_directories(site_logger PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(site_logger PUBLIC cxx_std_17)
ament_target_dependencies(site_logger
  diagnostic_msgs
  geometry_msgs
  rclcpp
  std_msgs
  std_srvs
)

install(TARGETS
  arm_planner
  bucket_controller
  proximity_detector
  truck_navigator
  payload_monitor
  load_dispatcher
  fleet_monitor
  site_logger
  DESTINATION lib/${PROJECT_NAME})

ament_package()