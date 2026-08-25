require "commonLoad.rb"
require "commonTaskParams.rb"
require "interfaces_CpmCommon.rb"
require "Robot.rb"

Interfaces = CommonTaskParams::Interfaces.dup

Interfaces.update({
  "SwitchInputScsInput" => InterfaceDefs_CpmCommon::SwitchInputScsInput,
})

Parameters = CommonTaskParams::Parameters.dup

Parameters.update({
  "loggerThreshold"   => "info",
  "scheduler"         => "SCHED_RR",
  "schedulerPriority" => 1,
})

Execution = {
  "executableName" => "ROS2BridgeApp",
}
