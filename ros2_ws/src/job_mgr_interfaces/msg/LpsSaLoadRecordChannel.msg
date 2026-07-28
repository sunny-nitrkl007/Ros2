# Wire equivalent of LpsSaLoadRecordChannelStorage (published as
# LoadRecordOutput, live call site LpsSaJobMgrProcess.cpp:224 ->publish()).
# Consumed by LpsSaTotalsApp/WorkOrderAssist via the Bridge -- neither is
# migrated, so this channel is Bridge-only, no direct DDS leg.
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaLoadRecordChannel/LpsSaLoadRecordChannel.h
#
# serialize() (lines 738-826) uses version-gated conditional archiving for
# backwards file-compatibility with older stored blobs -- that's an SCS/Boost
# concern for reading OLD data, not relevant to defining a NEW wire format.
# This message reflects the CURRENT field set (version 12, the max), i.e.
# every field the class actually declares.
#
# `subtotal_` (the struct's single "first" subtotal) is NOT fully re-archived
# in the outer serialize() (only startTime/endTime/truckId/.../passes_ are
# individually re-listed there, missing weightTonnes_/accuracy_/
# targetProportion/targetPasses/iconType -- likely legacy-compat scaffolding
# predating those fields). Modeled here as the FULL LoadRecordSubtotal type
# regardless: when the shim publishes live data it fills in the complete
# current in-memory value, so this is more correct for the new system than
# replicating that old partial-archive quirk.
#
# `weightUnits_` (LpsCommonWeightUnits): BLOCKED, same missing-header
# situation as LpsSaWeighTxChannel.msg's dig_stat/cal_stat/etc -- carried as
# raw int32, no named constants. Confirmed real enumerator names (not
# values): KLB, TONNE, TON, LB, KG (AisJhm2CANProcess.cpp:292-310).

# LpsSaLoadRecordStoreAction (InterfaceCommon.h:4-10) -- values verified,
# explicit in source, not blocked.
uint8 STORE_ACTION_AUTO=0
uint8 STORE_ACTION_EXTERNAL=1
uint8 STORE_ACTION_UI=2
uint8 STORE_ACTION_TICKET_NOT_FOR_TRADE=3
uint8 STORE_ACTION_TICKET_LEGAL_FOR_TRADE=4
uint8 store_action

LoadRecordSubtotal subtotal
LoadRecordSubtotal[] subtotals

uint32 ticket_number
string ticket_id

string product_id
string equipment_id
float32 weight_interval
int32 weight_decimal_precision
int32 weight_units    # LpsCommonWeightUnits -- blocked

string recipe_name

# LpsSaLoadRecordTargetType (InterfaceCommon.h:13-20) -- values verified,
# explicit in source, not blocked.
uint8 TARGET_TYPE_SINGLE=0
uint8 TARGET_TYPE_SPLIT=1
uint8 TARGET_TYPE_BATCH=2
uint8 TARGET_TYPE_MIX=3
uint8 TARGET_TYPE_BLEND_LOADS=4
uint8 TARGET_TYPE_BLEND_WEIGHT=5
uint8 target_type

float32 total_target_weight
uint16 current_subtotal_index

LoadRecordTimeStamp store_time
