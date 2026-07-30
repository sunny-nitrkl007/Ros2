################################################################################
# LpsSaLoadRecordStoreAction
################################################################################

uint8 STORE_ACTION_AUTO=0
uint8 STORE_ACTION_EXTERNAL=1
uint8 STORE_ACTION_UI=2
uint8 STORE_ACTION_TICKET_NOT_FOR_TRADE=3
uint8 STORE_ACTION_TICKET_LEGAL_FOR_TRADE=4

################################################################################
# LpsSaLoadRecordTargetType
################################################################################

uint8 TARGET_TYPE_SINGLE=0
uint8 TARGET_TYPE_SPLIT=1
uint8 TARGET_TYPE_BATCH=2
uint8 TARGET_TYPE_MIX=3
uint8 TARGET_TYPE_BLEND_LOADS=4
uint8 TARGET_TYPE_BLEND_WEIGHT=5

################################################################################
# LpsSaLoadRecordChannelStorage
################################################################################

uint8 store_action

LoadRecordSubtotal subtotal
LoadRecordSubtotal[] subtotals

uint32 ticket_number
string ticket_id

string product_id
string equipment_id
float32 weight_interval
int32 weight_decimal_precision
int32 weight_units

string recipe_name
uint8 target_type

float32 total_target_weight
uint16 current_subtotal_index

LoadRecordTimeStamp store_time
