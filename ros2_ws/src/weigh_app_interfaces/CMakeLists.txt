cmake_minimum_required(VERSION 3.8)
project(weigh_app_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(cpm_common_interfaces REQUIRED)
find_package(job_mgr_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  # PrinterCnfgInput (channel 1 of 15 -- see WEIGH_CHANNEL_MAPPING.md)
  "msg/LpsSaTotalsPrinterCnfgTruckTicket.msg"
  "msg/LpsSaTotalsPrinterCnfgTruckReport.msg"
  "msg/LpsSaTotalsPrinterCnfgMaterialReport.msg"
  "msg/LpsSaTotalsPrinterCnfg.msg"
  "msg/LpsSaTotalsPrinterCnfgInterface.msg"
  # SystemHardwareHealthInput (channel 2)
  "msg/DiskSpaceRemainType.msg"
  "msg/EthernetStatsType.msg"
  "msg/ECMVersionInfoType.msg"
  "msg/SystemHardwareHealthCoreUsage.msg"
  "msg/SystemHardwareHealth.msg"
  # SystemHardwareHealthRequestOutput (channel 15)
  "msg/SystemHardwareHealthRequest.msg"
  # PartNumbersInput (channel 3)
  "msg/PartNumbers.msg"
  # DataLinkDataInput (channel 4)
  "msg/DataLinkParam.msg"
  "msg/DataLinkData.msg"
  # ReadyToFlashStatusOutput (channel 5)
  "msg/ReadyToFlashStatus.msg"
  # LpsSaWeighInitDebugChannelOutput (channel 6)
  "msg/LpsSaWeighInitDebugChannel.msg"
  # LpsSaWeighDebugChannelOutput (channel 7)
  "msg/LpsSaWeighDebugChannel.msg"
  # PwmInputChannelsInput (channel 8)
  "msg/PwmInputChannels.msg"
  # MachineInput (channel 9)
  "msg/Machine.msg"
  # DemoAppTxChannelInput (channel 10)
  "msg/DemoAppTxChannel.msg"
  # CalMgrCmdReqstInput (channel 11)
  "msg/CalMgrCalibrationRequest.msg"
  "msg/CalMgrCmdReqst.msg"
  # CalMgrCmdRespOutput (channel 12)
  "msg/CalMgrCalibrationResp.msg"
  "msg/CalMgrCmdResp.msg"
  # LpsSaNvmCalDataChannelOutput (channel 13)
  "msg/LpsCalIMUResults.msg"
  "msg/LpsSaNvmCalDataChannelDataMain.msg"
  "msg/LpsSaNvmCalDataChannelDataDebug.msg"
  "msg/LpsSaNvmCalDataChannel.msg"
  # LpsSaNvmCalOnTheFlyDataChannelOutput (channel 14)
  "msg/LpsSaNvmCalOnTheFlyStatusFlags.msg"
  "msg/LpsSaNvmCalOnTheFlyCurveFitInfo.msg"
  "msg/LpsSaNvmCalOnTheFlyMiscUpdates.msg"
  "msg/LpsSaNvmCalOnTheFlyTiltOverrides.msg"
  "msg/LpsSaNvmCalOnTheFlyOverrides.msg"
  "msg/LpsSaNvmCalOnTheFlyDataOnTheFly.msg"
  "msg/LpsSaNvmCalOnTheFlyDataChannel.msg"
  DEPENDENCIES cpm_common_interfaces job_mgr_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
