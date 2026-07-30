################################################################################
# ToaAnchorStatus_t
################################################################################

uint8 TOA_ANCHOR_UNCHANGED=0
uint8 TOA_ANCHOR_RESET=1
uint8 TOA_ANCHOR_ACCEPTED=2
uint8 TOA_ANCHOR_REJECTED=3

################################################################################
# TestStatus_t
################################################################################

uint8 TEST_INACTIVE=0
uint8 TEST_TESTING=1
uint8 TEST_RECORDING=2
uint8 TEST_FAILED=3

################################################################################
# LpsSaWeighTxChannelStorage
################################################################################

int64 time_point_ns

int32 dig_stat
int32 cal_stat
int32 dump_stat

float32 best_bkt_wt_in_tonnes
uint16 warmup_lifts_required
uint16 payload_calc_meth
bool bkt_wt_latched_flag
bool latch_conditions_met
bool zero_available

Payload payload
LftSealStatus lft_seal_status
bool flash_enabled

string lift_position_sensor_id
string tilt_position_sensor_id
string lift_head_end_pressure_sensor_id
string lift_rod_end_pressure_sensor_id
string hydraulic_oil_temperature_sensor_id
string imu_sensor_id
string implement_serial_num
string work_tool_id

LiftPosition lift_position
FloatIO lift_cyl_vel
TiltPosition tilt_position

WeighRange weigh_range
int32 indicator
float32 cal_wt
bool lift_stalled

FloatIO lift_valve_command
FloatIO tilt_valve_command

AcdEventPopUp event_state
AcdDiagPopUp diag_state
AcdInfoPopUp info_state
bool show_exclamation_point
bool excessive_pitch
bool bucket_fully_racked

float32 zero_weight
float32 simple_cal_adjust

int16 engine_speed_rpm

WeighPidData pid_data

bool can11_message_timeout_flag

float32 toa_anchored_zero_offset
float32 toa_anchored_factor
uint8 toa_anchor_status

bool payload_cal_in_progress
uint8 test_status
