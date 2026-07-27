cmake_minimum_required(VERSION 3.8)
project(job_mgr_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(cpm_common_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/DispBestBktWt.msg"
  "msg/SimpleCalData.msg"
  "msg/LpsSaJobMgrTxChannel.msg"
  "msg/LpsSaJobMgrRespChannel.msg"
  "msg/LpsSaJobMgrDebugChannel.msg"
  DEPENDENCIES cpm_common_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
