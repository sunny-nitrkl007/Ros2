cmake_minimum_required(VERSION 3.8)
project(planning_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(adas_interfaces REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(planning1_node src/planning1_node.cpp)
target_include_directories(planning1_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(planning1_node PUBLIC cxx_std_17)
ament_target_dependencies(planning1_node
  adas_interfaces
  rclcpp
)

add_executable(planning2_node src/planning2_node.cpp)
target_include_directories(planning2_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(planning2_node PUBLIC cxx_std_17)
ament_target_dependencies(planning2_node
  adas_interfaces
  rclcpp
)

install(TARGETS
  planning1_node
  planning2_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()