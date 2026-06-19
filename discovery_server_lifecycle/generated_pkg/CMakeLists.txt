cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(lifecycle_msgs REQUIRED)

find_package(rclcpp REQUIRED)

find_package(rclcpp_lifecycle REQUIRED)

find_package(std_msgs REQUIRED)


include_directories(include)


add_executable(lifecycle_talker src/lifecycle_talker.cpp)
ament_target_dependencies(lifecycle_talker

  lifecycle_msgs

  rclcpp

  rclcpp_lifecycle

  std_msgs

)

add_executable(lifecycle_listener src/lifecycle_listener.cpp)
ament_target_dependencies(lifecycle_listener

  lifecycle_msgs

  rclcpp

  rclcpp_lifecycle

  std_msgs

)


install(TARGETS

  lifecycle_talker

  lifecycle_listener

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()