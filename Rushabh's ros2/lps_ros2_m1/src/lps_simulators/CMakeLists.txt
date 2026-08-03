cmake_minimum_required(VERSION 3.8)
project(lps_simulators)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(lps_interfaces REQUIRED)
find_package(std_srvs REQUIRED)
add_executable(lps_pass_tracker_scenario_node src/pass_tracker_scenario_node.cpp)
target_compile_features(lps_pass_tracker_scenario_node PRIVATE cxx_std_17)
ament_target_dependencies(lps_pass_tracker_scenario_node rclcpp lps_interfaces std_srvs)
install(TARGETS lps_pass_tracker_scenario_node DESTINATION lib/${PROJECT_NAME})
install(PROGRAMS
  scripts/pass_tracker_validation_node.py
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
ament_package()
