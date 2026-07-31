cmake_minimum_required(VERSION 3.8)
project(cpm_common_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
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
  "msg/FloatIO.msg"
  "msg/LiftPosition.msg"
  "msg/TiltPosition.msg"
  "msg/Payload.msg"
  "msg/LftSealStatus.msg"
  "msg/WeighRange.msg"
  "msg/ProdMeasureSensorStatus.msg"
  "msg/LinkSensorCalLim.msg"
  "msg/WeighPidData.msg"
  "msg/AcdInfoPopUp.msg"
  "msg/AcdDiagPopUp.msg"
  "msg/AcdEventPopUp.msg"
  "msg/LpsSaWeighTxChannel.msg"
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
