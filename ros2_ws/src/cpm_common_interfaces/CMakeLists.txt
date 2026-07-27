cmake_minimum_required(VERSION 3.8)
project(cpm_common_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/WeighBktWtAccuracy.msg"
  "msg/TipOffTriggerType.msg"
  "msg/TipOffState.msg"
  "msg/StandbyState.msg"
  "msg/JobMgrReqstChannelCommand.msg"
  "msg/WeighReqstChannelCommand.msg"
  "msg/FloatPair.msg"
  "msg/LpsSaJobMgrReqst.msg"
  "msg/LpsSaJobMgrSubtotalInfo.msg"
  "msg/LpsSaJobMgrReqstChannel.msg"
  "msg/LpsSaWeighReqstChannel.msg"
  "msg/LpsSaWeighRespChannel.msg"
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
