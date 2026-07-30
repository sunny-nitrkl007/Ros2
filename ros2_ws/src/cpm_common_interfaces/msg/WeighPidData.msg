################################################################################
# PloadSysZeroStat_t
################################################################################

uint16 ZERO_STAT_ZEROED=916
uint16 ZERO_STAT_NOT_ZEROED=917

################################################################################
# PloadSysCalWtEntryReqStat_t
################################################################################

uint16 CAL_WT_ENTRY_REQUIRED=26
uint16 CAL_WT_ENTRY_NOT_REQUIRED=27

################################################################################
# ACDWeighStatus::type
################################################################################

uint16 WEIGH_STATUS_BIT_LOWER_STALL=7
uint16 WEIGH_STATUS_BIT_RAISE_STALL=8
uint16 WEIGH_STATUS_BIT_INSUFFICIENT_DATA=9
uint16 WEIGH_STATUS_BIT_REWEIGH_PRESSURE_CHANGING=10
uint16 WEIGH_STATUS_BIT_REWEIGH_INCONSISTENT=11
uint16 WEIGH_STATUS_BIT_REWEIGH_SPEED_CHANGING=12
uint16 WEIGH_STATUS_BIT_REWEIGH_NOT_RACKED_EXCESSIVE_PITCH=13
uint16 WEIGH_STATUS_BIT_REWEIGH_STOPPED_IN_RANGE=14
uint16 WEIGH_STATUS_BIT_REWEIGH_LIFT_TOO_SLOW=15

################################################################################
# BktPayloadData_t
################################################################################

uint8 BKT_PAYLOAD_NOT_AVAILABLE=0
uint8 BKT_PAYLOAD_AVAILABLE=1
uint8 BKT_PAYLOAD_STORED=2
uint8 BKT_PAYLOAD_NOT_INSTALLED=244

################################################################################
# AudibleAnnunciationPriority_t
################################################################################

uint16 TONE_NONE=0
uint16 TONE_CONFIRMATION=2
uint16 TONE_ATTENTION=6

################################################################################
# WeighPidData_t
################################################################################

uint16 pload_sys_zero_stat

uint16 ldr_payload_stat
bool overload_warning_enabled

ProdMeasureSensorStatus prod_measure_sensor_status
LinkSensorCalLim link_sensor_cal_lim

float32 loader_bkt_pload_tgt_wt
float32 loader_bkt_pload_tgt_wt_per
uint16 pload_sys_zero_req_stat
uint16 pload_sys_cal_wt_entry_req_stat

float32 last_pload_wt

uint16 prod_measure_weigh_status
uint8 bkt_payload_data

int16 qr_hyd_oil_temp_min_c
int16 qr_lift_cyl_vel_min_mm_sec
int16 qr_lift_cyl_vel_max_mm_sec

int16 machine_rear_lateral_acceleration
int16 machine_rear_longitudinal_acceleration
int16 machine_rear_vertical_acceleration

int16 machine_pitch
int16 machine_slope
int16 machine_rear_roll
int16 machine_rear_side_slope
int16 machine_roll
int16 machine_side_slope

float32 tipoff_pitch_cal_offset
bool hyd_oil_temp_enabled
bool audible_weight_enabled
uint16 audible_weight_command
