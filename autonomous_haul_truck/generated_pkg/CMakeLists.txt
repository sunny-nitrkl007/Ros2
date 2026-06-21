cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)

add_executable(gps_imu_fusion src/gps_imu_fusion.cpp)
target_include_directories(gps_imu_fusion PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(gps_imu_fusion PUBLIC cxx_std_17)
ament_target_dependencies(gps_imu_fusion
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_srvs
)

add_executable(lidar_processor src/lidar_processor.cpp)
target_include_directories(lidar_processor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(lidar_processor PUBLIC cxx_std_17)
ament_target_dependencies(lidar_processor
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_srvs
)

add_executable(route_planner src/route_planner.cpp)
target_include_directories(route_planner PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(route_planner PUBLIC cxx_std_17)
ament_target_dependencies(route_planner
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_srvs
)

add_executable(vehicle_controller src/vehicle_controller.cpp)
target_include_directories(vehicle_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(vehicle_controller PUBLIC cxx_std_17)
ament_target_dependencies(vehicle_controller
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_srvs
)

add_executable(hazard_detector src/hazard_detector.cpp)
target_include_directories(hazard_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(hazard_detector PUBLIC cxx_std_17)
ament_target_dependencies(hazard_detector
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_srvs
)

add_executable(truck_health_monitor src/truck_health_monitor.cpp)
target_include_directories(truck_health_monitor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(truck_health_monitor PUBLIC cxx_std_17)
ament_target_dependencies(truck_health_monitor
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_srvs
)

install(TARGETS
  gps_imu_fusion
  lidar_processor
  route_planner
  vehicle_controller
  hazard_detector
  truck_health_monitor
  DESTINATION lib/${PROJECT_NAME})

ament_package()