cmake_minimum_required(VERSION 3.8)
project(weigh_app_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(cpm_common_interfaces REQUIRED)
find_package(job_mgr_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  # No .msg files yet -- see WEIGH_CHANNEL_MAPPING.md for the channel
  # inventory and per-channel status. Files get added here as each
  # channel is traced and drafted, same process as job_mgr_interfaces.
  DEPENDENCIES cpm_common_interfaces job_mgr_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
