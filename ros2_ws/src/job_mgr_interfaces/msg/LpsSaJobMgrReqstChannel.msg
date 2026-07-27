# Wire equivalent of LpsSaJobMgrReqstChannelStorage (consumed as
# LpsSaJobMgrReqstChannelInput, live call site LpsSaJobMgrScs.cpp:154).
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrReqstChannel/LpsSaJobMgrReqstChannel.h
#
# HYBRID CHANNEL (Development-Plan.txt Step 2.5 / Step 6.1.4): this topic has
# two independent publishers -- the legacy UI (aisJhm2RequestProc, via
# LpsSaJobMgrRequestHelper.hpp, relayed through the Bridge) and WeighApp
# (LpsSaScsSendZeroRqst(), direct DDS, confirmed live at LpsSaScs.cpp:700-712
# in LpsSaWeighApp). Both publish onto this same message type.
#
# `timePoint` dropped -- same reasoning as LpsSaJobMgrTxChannel.msg.

string app_name
uint32 app_request_id

# Command (LpsSaJobMgrReqstChannel.h:90-129) -- values verified, plain
# sequential enum class, 0..37 in declaration order, no explicit values in
# source.
uint8 CMD_NONE=0
uint8 CMD_ZERO=1
uint8 CMD_MINUS_ONE=2
uint8 CMD_CLEAR=3
uint8 CMD_STORE=4
uint8 CMD_REWEIGH=5
uint8 CMD_MANUAL_ADD=6
uint8 CMD_STANDBY_ACTIVATE=7
uint8 CMD_STANDBY_DEACTIVATE=8
uint8 CMD_TIPOFF_MODE_TOGGLE=9
uint8 CMD_MANUAL_TIPOFF_ACTIVATE=10
uint8 CMD_MANUAL_TIPOFF_DEACTIVATE=11
uint8 CMD_WRITE_TIPOFF_TRIGGER_TYPE=12
uint8 CMD_WRITE_TIPOFF_MODE=13
uint8 CMD_WRITE_HORN_ON_STORE_ENABLED=14
uint8 CMD_WRITE_AUTO_STORE_PASS_COUNT=15
uint8 CMD_WRITE_AUTO_TRUCK_ID_ENABLED=16
uint8 CMD_WRITE_AUTO_MATERIAL_ID_ENABLED=17
uint8 CMD_WRITE_MANUAL_ADD_ENABLED=18
uint8 CMD_WRITE_MULTI_TASK_ENABLED=19
uint8 CMD_WRITE_MULTI_TASK_COUNT=20
uint8 CMD_WRITE_SPLIT_MODE_ENABLED=21
uint8 CMD_SELECT_TASK=22
uint8 CMD_SELECT_NEXT_TASK=23
uint8 CMD_SELECT_PREVIOUS_TASK=24
uint8 CMD_WRITE_TRUCK_LIST_ENABLED=25
uint8 CMD_WRITE_MATERIAL_LIST_ENABLED=26
uint8 CMD_WRITE_TAG1_ENABLED=27
uint8 CMD_WRITE_TAG2_ENABLED=28
uint8 CMD_WRITE_TAG3_ENABLED=29
uint8 CMD_WRITE_TAG4_ENABLED=30
uint8 CMD_WRITE_PAYLOAD_NEXT_SUBTOTAL=31
uint8 CMD_WRITE_LFT_DISABLED=32
uint8 CMD_WRITE_TARGET_TYPE=33
uint8 CMD_WRITE_TOTAL_TARGET_WEIGHT=34
uint8 CMD_SELECT_SUBTOTAL=35
uint8 CMD_PAYLOAD_DETAILS_FILE_REQUEST=36
uint8 CMD_WRITE_SUBTOTAL_INFO=37
uint8 command

# `data` sub-struct (LpsSaJobMgrReqstChannel.h:149-159) -- flattened with a
# data_ prefix since ROS2 .msg has no anonymous-struct equivalent.
TipOffTriggerType data_tipoff_trigger_type
TipOffState data_tipoff_mode
uint16 data_auto_store_pass_count
uint8 data_task_number
bool data_enabled
uint8 data_target_type
uint16 data_subtotal_index
float32 data_total_target_weight
LpsSaJobMgrSubtotalInfo data_subtotal_info

LpsSaJobMgrReqst[] requests
