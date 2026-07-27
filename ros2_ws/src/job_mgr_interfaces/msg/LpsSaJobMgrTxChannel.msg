
# `timePoint` (std::chrono::steady_clock::time_point in the original struct)
# is dropped: verified via repo-wide grep that no consumer anywhere reads it
# back, only the publisher's own constructor/serialize() touch it.

# WeighBktWtAccuracy, TipOffTriggerType, TipOffState, StandbyState now live in
# cpm_common_interfaces (moved out of this package): each is used directly by
# WeighApp's own code too, not just via a JobMgr channel struct -- see each
# type's own file header in cpm_common_interfaces/msg/ for the specific
# evidence that justified the move.

uint8 task_number
uint16 pass_count
float32 truck_weight
cpm_common_interfaces/WeighBktWtAccuracy truck_weight_accuracy
float32 truck_start_weight
float32 remaining_weight
float32 total_weight
cpm_common_interfaces/WeighBktWtAccuracy total_weight_accuracy
uint32 subtotal_count
string ticket_id

uint32 material_id
string material_name
float32 material_density

uint32 truck_id
string truck_name
float32 truck_target_weight

bool truck_list_enabled
bool material_list_enabled

bool tag1_enabled
string custom_list_name1
string tag1
bool tag2_enabled
string custom_list_name2
string tag2
bool tag3_enabled
string custom_list_name3
string tag3
bool tag4_enabled
string custom_list_name4
string tag4

# LpsSaJobMgrManualTipOffState_t (LpsSaJobMgrTxChannel.h:32-38) 
uint8 MAN_TIP_OFF_AVAILABLE=0
uint8 MAN_TIP_OFF_UNAVAILABLE=1
uint8 MAN_TIP_OFF_ACTIVE=2
uint8 MAN_TIP_OFF_INACTIVE=3
uint8 manual_tip_off_state

cpm_common_interfaces/TipOffTriggerType tip_off_trigger_type

# Shared by tip_off_state and tip_off_state_cfg below (same C++ type used for
# both fields in the original LpsSaJobMgrTxChannelStorage).
cpm_common_interfaces/TipOffState tip_off_state
cpm_common_interfaces/TipOffState tip_off_state_cfg

# LpsSaJobMgrOperationMode_t (LpsSaJobMgrTxChannel.h:40-48) ,
# note >255 so uint16 not uint8
uint16 OP_MODE_UNCALIBRATED=838
uint16 OP_MODE_WEIGH=9
uint16 OP_MODE_STANDBY=175
uint16 OP_MODE_EXCESS=846
uint16 operation_mode

cpm_common_interfaces/StandbyState standby_state

# LpsSaJobMgrClearMinusOneEnableStat_t (LpsSaJobMgrTxChannel.h:56-60) 
uint8 CLEAR_BTN_ENABLED=0
uint8 MINUS_ONE_BTN_ENABLED=1
uint8 clear_minus_one_enable_stat

DispBestBktWt disp_best_bkt_wt

SimpleCalData[] simple_cal_data
uint32 store_count
bool store_rejected

# ReqPloadCtrlSysStat_t (LpsSaJobMgrTxChannel.h:82-91) , uint16
uint16 CTRL_STAT_NONE=58
uint16 CTRL_STAT_CLEAR=551
uint16 CTRL_STAT_REWEIGH=763
uint16 CTRL_STAT_ZERO=862
uint16 CTRL_STAT_STORE=863
uint16 CTRL_STAT_TIPOFFTOGGLE=1438
uint16 CTRL_STAT_MINUS_ONE=1584
uint16 req_pload_ctrl_sys_stat

# StorePloadHornStat_t (LpsSaJobMgrTxChannel.h:102-106) , uint16
uint16 HORN_SOUND=794
uint16 HORN_NOT_SOUND=795
uint16 horn_store_state

uint16 auto_store_pass_count
bool auto_truck_id_enabled
bool auto_material_id_enabled
bool manual_add_enabled
bool multi_task_enabled
uint8 multi_task_count
bool tipoff_active
bool tipoff_assist_active
uint16 tipoff_assist_active_eid
bool manual_add_available
bool split_mode_enabled
bool lft_disabled
uint8 target_type
uint16 step_number
uint8 icon_type
uint16 target_passes
