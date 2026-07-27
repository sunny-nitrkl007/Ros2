cmake_minimum_required(VERSION 3.8)
project(job_mgr_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/WeighBktWtAccuracy.msg"
  "msg/DispBestBktWt.msg"
  "msg/SimpleCalData.msg"
  "msg/TipOffTriggerType.msg"
  "msg/TipOffState.msg"
  "msg/JobMgrReqstChannelCommand.msg"
  "msg/LpsSaJobMgrTxChannel.msg"
  "msg/LpsSaJobMgrReqst.msg"
  "msg/LpsSaJobMgrSubtotalInfo.msg"
  "msg/LpsSaJobMgrReqstChannel.msg"
  "msg/LpsSaJobMgrRespChannel.msg"
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
