# Wire equivalent of SystemHardwareHealthStorage (SystemHardwareHealthInput
# channel, WeighApp's SystemHardwareHealthInput_).
# Source: eta-ais/prod/coretech/ais_pcm/ais/interfaces/SystemHardwareHealth/SystemHardwareHealth.h:111-548
# Field presence cross-checked against serialize() (lines 211-378) -- takes
# the highest-numbered case (9, the current BOOST_CLASS_VERSION) as the
# full current field set; case is a plain switch (not cumulative
# `if (version >= N)` gating like other channels), but case 9 itself
# repeats every field from every earlier case in sequence, so it IS the
# complete superset.
#
# listOfEthernetStats is declared as a member (SystemHardwareHealth.h:155)
# but only actually archived starting at case 9 (line 369) -- included
# since case 9 is the version modeled.

string source_system
uint32 total_system_memory_kb
uint32 available_system_memory_kb
DiskSpaceRemainType[] list_of_remaining_disk_space
float64 system_up_time_sec
float64 load_avg1min
float64 load_avg5min
float64 load_avg15min
string[] core_temperatures_c
string ecm_sw_version
string software_group_part_number
string software_group_description
string software_group_release_date
ECMVersionInfoType ecm_hw_version
float64 eth0_tx_kbs
float64 eth0_rx_kbs
float64 eth1_tx_kbs
float64 eth1_rx_kbs
EthernetStatsType[] list_of_ethernet_stats
float64 system_cpu_usage_percent
float64 system_cpu_io_wait_percent
float64 system_cpu_irq_percent
string ecm_os_version
SystemHardwareHealthCoreUsage[] per_core_cpu_data
float64 battery_voltage
