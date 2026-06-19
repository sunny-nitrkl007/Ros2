cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)

find_package(std_srvs REQUIRED)


include_directories(include)


add_executable(reset_server src/reset_server.cpp)
ament_target_dependencies(reset_server

  rclcpp

  std_srvs

)

add_executable(reset_client src/reset_client.cpp)
ament_target_dependencies(reset_client

  rclcpp

  std_srvs

)


install(TARGETS

  reset_server

  reset_client

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()