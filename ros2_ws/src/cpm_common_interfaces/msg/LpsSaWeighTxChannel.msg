# Wire equivalent of LpsSaWeighTxChannelStorage (published by WeighApp as
# LpsSaWeighTxChannelOutput; consumed by JobMgr as LpsSaWeighTxChannelInput,
# live call site LpsSaJobMgrScs.cpp:849 via weighAppInf_.waitForTxData()).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaWeighTxChannel/LpsSaWeighTxChannel.h
#
# DIRECT DDS CHANNEL (Development-Plan.txt Step 6.1.3): WeighApp -> JobMgr,
# no Bridge involved.
#
# Field presence cross-checked against serialize() (LpsSaWeighTxChannel.h:429-563)
# -- every declared field is archived, including ImplementSerialNum which is
# serialized separately at the very end (line 562), easy to miss.
#
# `timePoint` IS live -- LpsSaWeighAppInf.hpp:270,289 compares this against
# LpsSaWeighRespChannel's timePoint for response/tx freshness correlation.
# Same int64-nanosecond representation as the other timePoint fields.
#
# 6 fields (dig_stat, cal_stat, dump_stat, indicator, and the two nested
# FloatIO/LiftPosition/TiltPosition/Payload `status` fields) reference types
# from the missing LpsPublic.h -- BLOCKED, carried as raw ints with no named
# constants. See FloatIO.msg for the full explanation; same reasoning here,
# not repeated per-field.
#
# ToaAnchorStatus_t and TestStatus_t (below) ARE locally defined in this same
# header (LpsSaWeighTxChannel.h:214-238) -- values fully verified, not blocked.

int64 time_point_ns

int32 dig_stat    # LpsWeighBktDigStat_t -- blocked
int32 cal_stat    # LpsWeighCalStatus_t -- blocked
int32 dump_stat   # LpsWeighBktDumpStat_t -- blocked

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
int32 indicator   # LpsWeighRangeIndicator_t -- blocked
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

# ToaAnchorStatus_t (LpsSaWeighTxChannel.h:214-219) -- values verified,
# plain sequential enum, locally defined, not blocked.
uint8 TOA_ANCHOR_UNCHANGED=0
uint8 TOA_ANCHOR_RESET=1
uint8 TOA_ANCHOR_ACCEPTED=2
uint8 TOA_ANCHOR_REJECTED=3
uint8 toa_anchor_status

bool payload_cal_in_progress

# TestStatus_t (LpsSaWeighTxChannel.h:233-238) -- values verified, plain
# sequential enum, locally defined, not blocked.
uint8 TEST_INACTIVE=0
uint8 TEST_TESTING=1
uint8 TEST_RECORDING=2
uint8 TEST_FAILED=3
uint8 test_status
