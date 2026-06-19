cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(example_interfaces REQUIRED)

find_package(rclcpp REQUIRED)

find_package(rclcpp_action REQUIRED)


include_directories(include)


add_executable(fibonacci_server src/fibonacci_server.cpp)
ament_target_dependencies(fibonacci_server

  example_interfaces

  rclcpp

  rclcpp_action

)

add_executable(fibonacci_client src/fibonacci_client.cpp)
ament_target_dependencies(fibonacci_client

  example_interfaces

  rclcpp

  rclcpp_action

)


install(TARGETS

  fibonacci_server

  fibonacci_client

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()