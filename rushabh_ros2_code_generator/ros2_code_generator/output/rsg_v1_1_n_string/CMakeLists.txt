cmake_minimum_required(VERSION 3.8)
project(rsg_v1_1_n_string)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)


add_executable(sender
  src/sender.cpp
)
target_include_directories(sender PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(sender
  rclcpp
  std_msgs
)
add_executable(receiver_1
  src/receiver_1.cpp
)
target_include_directories(receiver_1 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(receiver_1
  rclcpp
  std_msgs
)
add_executable(receiver_2
  src/receiver_2.cpp
)
target_include_directories(receiver_2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(receiver_2
  rclcpp
  std_msgs
)
add_executable(receiver_3
  src/receiver_3.cpp
)
target_include_directories(receiver_3 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(receiver_3
  rclcpp
  std_msgs
)

install(TARGETS
  sender
  receiver_1
  receiver_2
  receiver_3
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
