cmake_minimum_required(VERSION 3.8)
project(generated_pkg)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)

find_package(custom_interfaces_pkg REQUIRED)

find_package(rclcpp REQUIRED)


include_directories(include)


add_executable(sensor_publisher src/sensor_publisher.cpp)
ament_target_dependencies(sensor_publisher

  custom_interfaces_pkg

  rclcpp

)

add_executable(sensor_subscriber src/sensor_subscriber.cpp)
ament_target_dependencies(sensor_subscriber

  custom_interfaces_pkg

  rclcpp

)

add_executable(sensor_query_server src/sensor_query_server.cpp)
ament_target_dependencies(sensor_query_server

  custom_interfaces_pkg

  rclcpp

)

add_executable(sensor_query_client src/sensor_query_client.cpp)
ament_target_dependencies(sensor_query_client

  custom_interfaces_pkg

  rclcpp

)


install(TARGETS

  sensor_publisher

  sensor_subscriber

  sensor_query_server

  sensor_query_client

  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()