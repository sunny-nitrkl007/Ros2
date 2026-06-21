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
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(visualization_msgs REQUIRED)

add_executable(sensor_fusion src/sensor_fusion.cpp)
target_include_directories(sensor_fusion PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(sensor_fusion PUBLIC cxx_std_17)
ament_target_dependencies(sensor_fusion
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_msgs
  std_srvs
  visualization_msgs
)

add_executable(object_detector src/object_detector.cpp)
target_include_directories(object_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(object_detector PUBLIC cxx_std_17)
ament_target_dependencies(object_detector
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_msgs
  std_srvs
  visualization_msgs
)

add_executable(path_planner src/path_planner.cpp)
target_include_directories(path_planner PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(path_planner PUBLIC cxx_std_17)
ament_target_dependencies(path_planner
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_msgs
  std_srvs
  visualization_msgs
)

add_executable(decision_maker src/decision_maker.cpp)
target_include_directories(decision_maker PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(decision_maker PUBLIC cxx_std_17)
ament_target_dependencies(decision_maker
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_msgs
  std_srvs
  visualization_msgs
)

add_executable(safety_monitor src/safety_monitor.cpp)
target_include_directories(safety_monitor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(safety_monitor PUBLIC cxx_std_17)
ament_target_dependencies(safety_monitor
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_msgs
  std_srvs
  visualization_msgs
)

add_executable(watchdog src/watchdog.cpp)
target_include_directories(watchdog PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(watchdog PUBLIC cxx_std_17)
ament_target_dependencies(watchdog
  diagnostic_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  std_msgs
  std_srvs
  visualization_msgs
)

install(TARGETS
  sensor_fusion
  object_detector
  path_planner
  decision_maker
  safety_monitor
  watchdog
  DESTINATION lib/${PROJECT_NAME})

ament_package()