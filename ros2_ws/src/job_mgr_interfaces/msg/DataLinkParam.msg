# PARTIAL RECONSTRUCTION -- DataLinkParam. Real struct (folder
# interfaces/DataLinkData/, confirmed via LpsSaJobMgrApp.h:46) does not exist
# in this checkout. This is a large, generic, type-erased parameter system
# (confirmed by grepping its other consumer, autonomyConditionDiagnostics'
# DataLinkDataScsRx.cpp -- 55KB of matches, dozens of PIDs, multiple
# GetLastGoodValue<T> instantiations) -- NOT fully reconstructed here, only
# SCOPED to exactly what JobMgr itself reads (LpsSaJobMgrScs.cpp:763-837),
# which is 3 specific PIDs, each read through a different accessor:
#
#   STORE_BUTTON_PID   (0xF1AA)    -> GetLastGoodValue<uint8_t>()
#   TIPOFF_ASSSIST_PID (0xD118CE)  -> GetLastValueEng() (float -- printed
#                                     with %f at LpsSaJobMgrScs.cpp:810)
#   MANUAL_ADD_PID     (0xD11890)  -> GetLastGoodValue<uint16_t>()
#
# All three value fields are carried; the shim's conversion function picks
# the right one per param_id, mirroring what the real code already does.
# This does NOT model every value type the real generic system supports --
# only these three, because those are the only ones JobMgr itself ever asks
# for. If a future channel needs a PID with some other value type, add a
# field then, don't guess one now.
#
# identifier_type (DataLinkParam::ParamIdentifierType): only two enumerator
# NAMES seen anywhere in the tree, no numeric values --
# DATA_LINK_PARAM_IDENTIFIER_PID (LpsSaJobMgrScs.cpp:778) and
# DATA_LINK_PARAM_IDENTIFIER_CAT_EXT (DataLinkDataScsRx.cpp) -- raw uint8.
#
# last_value_dsi: compared against values up to 0xFF11 (65297) in
# DataLinkDataScsRx.cpp -- needs more than 16 bits of headroom, carried as
# uint32 to be safe rather than truncate real data.

uint32 param_id
uint8 identifier_type
bool pid_data_received
uint32 last_value_dsi

uint8 last_good_value_u8
uint16 last_good_value_u16
float32 last_value_eng
