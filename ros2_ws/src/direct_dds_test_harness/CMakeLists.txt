cmake_minimum_required(VERSION 3.8)
project(direct_dds_test_harness)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cpm_common_interfaces REQUIRED)

# Real, already-converted, AIS-free production headers reused directly:
#   - CPM-Loader-ais/prod/common/ros_shim/*.h            (shim layer)
#   - CPM-Loader-ais/prod/common/interfaces/LpsSaWeighReqstChannel/
#     LpsSaWeighAppInf.hpp                                (JobMgr's real
#     direct-DDS wrapper -- see Challenges-And-Decisions.txt 6.11)
# Neither has any AIS SDK dependency, which is what makes this harness
# buildable without lps_common/ais_task/etc.
set(AIS_REPO_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/../../../CPM-Loader-ais/prod/common")

add_executable(jobmgr_harness_node src/jobmgr_harness_node.cpp)
target_include_directories(jobmgr_harness_node PRIVATE ${AIS_REPO_COMMON})
ament_target_dependencies(jobmgr_harness_node rclcpp cpm_common_interfaces)

add_executable(weighapp_harness_node src/weighapp_harness_node.cpp)
target_include_directories(weighapp_harness_node PRIVATE ${AIS_REPO_COMMON})
ament_target_dependencies(weighapp_harness_node rclcpp cpm_common_interfaces)

install(TARGETS
  jobmgr_harness_node
  weighapp_harness_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
