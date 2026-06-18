cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)

find_package(std_msgs REQUIRED)


include_directories(include)


add_executable(talker_1 src/talker_1.cpp)
ament_target_dependencies(talker_1

  rclcpp

  std_msgs

)

add_executable(talker_2 src/talker_2.cpp)
ament_target_dependencies(talker_2

  rclcpp

  std_msgs

)

add_executable(talker_3 src/talker_3.cpp)
ament_target_dependencies(talker_3

  rclcpp

  std_msgs

)

add_executable(listener src/listener.cpp)
ament_target_dependencies(listener

  rclcpp

  std_msgs

)


install(TARGETS

  talker_1

  talker_2

  talker_3

  listener

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()