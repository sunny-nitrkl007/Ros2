# Wire equivalent of LpsSaUIDisplayState (consumed as DisplayStateInput,
# live call site LpsSaJobMgrScs.cpp:138 ->get()). Config channel name is
# LpsSaUIDisplayState (config/interfaces_Loaders.rb:72), the struct itself
# is unrelated to the InterfaceTypes.h wrapper naming.
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaUI/DisplayState.hpp
#
# Field presence cross-checked against serialize() (lines 216-242) at
# current version 4 -- all fields present. legalForTradeDescriptiveMarkingCache_
# is explicitly marked "Do not serialize" in source and correctly excluded
# here -- it's a derived/cached string, not real wire data.

bool service_mode_enable_code_entered
bool in_service_mode
bool in_verification_mode
uint32 heartbeat_count
uint16 document_id
float32 weight_capacity
int32 weight_decimal_precision
float32 weight_interval
LpsSaUIDisplaySettings settings
