cmake_minimum_required(VERSION 3.8)
project(rsg_v1_n_1_msg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rosidl_default_runtime REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MachineStatus.msg"
  DEPENDENCIES builtin_interfaces
)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")

add_executable(sender_1
  src/sender_1.cpp
)
target_include_directories(sender_1 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(sender_1
  builtin_interfaces
  rclcpp
)
target_link_libraries(sender_1 "${cpp_typesupport_target}")
add_executable(sender_2
  src/sender_2.cpp
)
target_include_directories(sender_2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(sender_2
  builtin_interfaces
  rclcpp
)
target_link_libraries(sender_2 "${cpp_typesupport_target}")
add_executable(sender_3
  src/sender_3.cpp
)
target_include_directories(sender_3 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(sender_3
  builtin_interfaces
  rclcpp
)
target_link_libraries(sender_3 "${cpp_typesupport_target}")
add_executable(receiver
  src/receiver.cpp
)
target_include_directories(receiver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(receiver
  builtin_interfaces
  rclcpp
)
target_link_libraries(receiver "${cpp_typesupport_target}")

install(TARGETS
  sender_1
  sender_2
  sender_3
  receiver
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
