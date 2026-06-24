cmake_minimum_required(VERSION 3.8)
project(safety_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(adas_interfaces REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(safety_node src/safety_node.cpp)
target_include_directories(safety_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(safety_node PUBLIC cxx_std_17)
ament_target_dependencies(safety_node
  adas_interfaces
  rclcpp
)

install(TARGETS
  safety_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()