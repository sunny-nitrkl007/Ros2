################################################################################
# Weight Accuracy
################################################################################

uint8 WEIGHT_ACCURACY_NONE=0
uint8 WEIGHT_ACCURACY_LOW=1
uint8 WEIGHT_ACCURACY_MED=2
uint8 WEIGHT_ACCURACY_HIGH=3

################################################################################
# Tip Off Trigger Type
################################################################################

uint8 TIP_OFF_TRIGGER_AUTO=0
uint8 TIP_OFF_TRIGGER_MANUAL=1
uint8 TIP_OFF_TRIGGER_DISABLED=2

################################################################################
# Tip Off State
################################################################################

uint8 TIP_OFF_PILE_ENABLE=0
uint8 TIP_OFF_TRUCK_ENABLE=1
uint8 TIP_OFF_UNAVAILABLE=2

################################################################################
# Manual Tip Off State
################################################################################

uint8 MAN_TIP_OFF_AVAILABLE=0
uint8 MAN_TIP_OFF_UNAVAILABLE=1
uint8 MAN_TIP_OFF_ACTIVE=2
uint8 MAN_TIP_OFF_INACTIVE=3

################################################################################
# Payload Operating Mode
################################################################################

uint16 OP_MODE_UNCALIBRATED=0x0346
uint16 OP_MODE_WEIGH=0x0009
uint16 OP_MODE_STANDBY=0x00AF
uint16 OP_MODE_EXCESS=0x034E

################################################################################
# Standby State
################################################################################

uint8 STANDBY_ACTIVATED=0
uint8 STANDBY_DEACTIVATED=1

################################################################################
# Clear Minus One Enable Status
################################################################################

uint8 CLEAR_BTN_ENABLED=0
uint8 MINUS_ONE_BTN_ENABLED=1

################################################################################
# Payload Control System Status
################################################################################

uint16 PCS_NONE=0x003A
uint16 PCS_CLEAR=0x0227
uint16 PCS_REWEIGH=0x02FB
uint16 PCS_ZERO=0x035E
uint16 PCS_STORE=0x035F
uint16 PCS_TIPOFFTOGGLE=0x059E
uint16 PCS_MINUS_ONE=0x0630

################################################################################
# Requested Payload Operating Mode
################################################################################

uint16 PLOAD_MODE_NORMAL=0x0009
uint16 PLOAD_MODE_STANDBY=0x00AF
uint16 PLOAD_MODE_NOT_CALIBRATED=0x0346
uint16 PLOAD_MODE_EXCESS=0x034E
uint16 PLOAD_MODE_TRUCK_WEIGHT_ADJ=0x034F

################################################################################
# Horn State
################################################################################

uint16 SOUND_HORN=0x031A
uint16 NOT_SOUND_HORN=0x031B

################################################################################
# Data
################################################################################

builtin_interfaces/Time time_point
uint8 task_number
uint16 pass_count
float32 truck_weight
uint8 truck_weight_accuracy
float32 truck_start_weight
float32 remaining_weight
float32 total_weight
uint8 total_weight_accuracy
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
uint8 manual_tip_off_state
uint8 tip_off_trigger_type
uint8 tip_off_state
uint16 operation_mode
uint8 standby_state
uint8 clear_minus_one_enable_stat
DispBestBktWt disp_best_bkt_wt
uint8 tip_off_state_cfg
SimpleCalData[] simple_cal_data
uint32 store_count
bool store_rejected
uint16 req_pload_ctrl_sys_stat
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
