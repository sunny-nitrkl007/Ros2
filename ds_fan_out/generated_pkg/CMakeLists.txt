cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)

find_package(std_msgs REQUIRED)


include_directories(include)


add_executable(talker src/talker.cpp)
ament_target_dependencies(talker

  rclcpp

  std_msgs

)

add_executable(listener_1 src/listener_1.cpp)
ament_target_dependencies(listener_1

  rclcpp

  std_msgs

)

add_executable(listener_2 src/listener_2.cpp)
ament_target_dependencies(listener_2

  rclcpp

  std_msgs

)

add_executable(listener_3 src/listener_3.cpp)
ament_target_dependencies(listener_3

  rclcpp

  std_msgs

)


install(TARGETS

  talker

  listener_1

  listener_2

  listener_3

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()