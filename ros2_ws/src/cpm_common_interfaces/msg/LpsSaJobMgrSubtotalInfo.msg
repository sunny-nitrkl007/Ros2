# Wire equivalent of LpsSaJobMgrReqstChannelStorage::LpsSaJobMgrSubtotalInfo_t.
# Source: LpsSaJobMgrReqstChannel.h:136-147
# Field named `command` in the original -- a plain string here, NOT the
# Command enum used elsewhere in this channel. Kept as subtotal_command to
# avoid any confusion with LpsSaJobMgrReqstChannel.msg's own `command` field.

string subtotal_command
uint16 current_step_number
uint16 new_step_number
float32 target_weight
uint16 target_proportion
uint16 target_passes
uint32 material_id
string material_name
float32 material_density
uint8 icon_type
