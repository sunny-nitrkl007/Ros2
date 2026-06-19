cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)

find_package(std_msgs REQUIRED)


include_directories(include)


add_executable(robot1_talker src/robot1_talker.cpp)
ament_target_dependencies(robot1_talker

  rclcpp

  std_msgs

)

add_executable(robot1_listener src/robot1_listener.cpp)
ament_target_dependencies(robot1_listener

  rclcpp

  std_msgs

)

add_executable(robot2_talker src/robot2_talker.cpp)
ament_target_dependencies(robot2_talker

  rclcpp

  std_msgs

)

add_executable(robot2_listener src/robot2_listener.cpp)
ament_target_dependencies(robot2_listener

  rclcpp

  std_msgs

)


install(TARGETS

  robot1_talker

  robot1_listener

  robot2_talker

  robot2_listener

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()