################################################################################
# SystemHardwareHealthStorage
################################################################################

string source_system
uint32 total_system_memory_kb
uint32 available_system_memory_kb
DiskSpaceRemain[] list_of_remaining_disk_space
float64 system_up_time_sec
float64 load_avg1min
float64 load_avg5min
float64 load_avg15min
string[] core_temperatures_c
string ecm_sw_version
string software_group_part_number
string software_group_description
string software_group_release_date
EcmVersionInfo ecm_hw_version
float64 eth0_tx_kbs
float64 eth0_rx_kbs
float64 eth1_tx_kbs
float64 eth1_rx_kbs
EthernetStatsType[] list_of_ethernet_stats
float64 system_cpu_usage_percent
float64 system_cpu_io_wait_percent
float64 system_cpu_irq_percent
string ecm_os_version
CoreUsage[] per_core_cpu_data
float64 battery_voltage
