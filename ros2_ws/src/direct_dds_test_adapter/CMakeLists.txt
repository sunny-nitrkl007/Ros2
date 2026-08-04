cmake_minimum_required(VERSION 3.8)
project(direct_dds_test_adapter)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cpm_common_interfaces REQUIRED)
find_package(job_mgr_interfaces REQUIRED)

# Real, already-converted, AIS-free production headers reused directly:
#   - CPM-Loader-ais/prod/common/ros2_wrapper/*.h            (wrapper layer)
#   - CPM-Loader-ais/prod/common/interfaces/LpsSaWeighReqstChannel/
#     DDSWeighAppInf.hpp                                  (JobMgr's real
#     direct-DDS wrapper -- see Challenges-And-Decisions.txt 6.11. NOT
#     LpsSaWeighAppInf.hpp, which stays untouched -- see that file's
#     own header comment for why)
# Neither has any AIS SDK dependency, which is what makes this adapter
# buildable without lps_common/ais_task/etc.
set(AIS_REPO_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/../../../CPM-Loader-ais/prod/common")

add_executable(jobmgr_adapter_node src/jobmgr_adapter_node.cpp)
target_include_directories(jobmgr_adapter_node PRIVATE ${AIS_REPO_COMMON})
ament_target_dependencies(jobmgr_adapter_node rclcpp cpm_common_interfaces job_mgr_interfaces)

add_executable(weighapp_adapter_node src/weighapp_adapter_node.cpp)
target_include_directories(weighapp_adapter_node PRIVATE ${AIS_REPO_COMMON})
ament_target_dependencies(weighapp_adapter_node rclcpp cpm_common_interfaces job_mgr_interfaces)

install(TARGETS
  jobmgr_adapter_node
  weighapp_adapter_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
