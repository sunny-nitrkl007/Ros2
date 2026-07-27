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

JobMgrReqstChannelCommand command

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
