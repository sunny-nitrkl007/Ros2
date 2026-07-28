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
  "msg/LoadRecordTimeStamp.msg"
  "msg/LoadRecordPass.msg"
  "msg/LoadRecordSubtotal.msg"
  "msg/LpsSaLoadRecordChannel.msg"
  "msg/SimpleCalDataT.msg"
  "msg/TipoffWeightAdjustData.msg"
  "msg/AisJhm2TxChannel.msg"
  "msg/LpsSaUIDisplaySettings.msg"
  "msg/LpsSaUIDisplayState.msg"
  "msg/ShmClockInput.msg"
  "msg/SEA.msg"
  "msg/AutonomyConditionDiagnosticsTxChannel.msg"
  "msg/J1939Name.msg"
  "msg/Diagnostic.msg"
  "msg/EventDiagnosticData.msg"
  DEPENDENCIES cpm_common_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
