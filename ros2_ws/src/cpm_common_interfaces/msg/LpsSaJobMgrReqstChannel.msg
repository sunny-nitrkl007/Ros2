################################################################################
# LpsSaJobMgrReqstChannelStorage
################################################################################

string app_name
uint32 app_request_id

JobMgrReqstChannelCommand command

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
