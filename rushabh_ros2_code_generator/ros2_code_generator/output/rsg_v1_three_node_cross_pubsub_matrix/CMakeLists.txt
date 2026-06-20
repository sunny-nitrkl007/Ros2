cmake_minimum_required(VERSION 3.8)
project(rsg_v1_three_node_cross_pubsub_matrix)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)


add_executable(node_a
  src/node_a.cpp
)
target_include_directories(node_a PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(node_a
  rclcpp
  std_msgs
)
add_executable(node_b
  src/node_b.cpp
)
target_include_directories(node_b PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(node_b
  rclcpp
  std_msgs
)
add_executable(node_c
  src/node_c.cpp
)
target_include_directories(node_c PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(node_c
  rclcpp
  std_msgs
)

install(TARGETS
  node_a
  node_b
  node_c
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
