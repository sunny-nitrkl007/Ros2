# Wire equivalent of ShmClockStorage (consumed as ShmClockInput, live call
# site LpsSaJobMgrScs.cpp:730 ->get()).
# Source: eta-ais/prod/machineCommon/content/prod/common/interfaces/ShmClock/ShmClock.h
# (NOT under CPM-Loader-ais/prod/common/interfaces/ like every other channel
# so far -- this one lives in the machineCommon submodule instead. Same true
# for DataLinkData and EventDiagnosticData, channels 14 and 16.)
#
# Field presence cross-checked against serialize() (lines 20-41) at current
# version 3 (the else branch, all 4 fields) -- ShmClock.h:113
# `BOOST_CLASS_VERSION(ShmClockStorage, 3)`.
#
# `utcSec` (TimeStamp, ais/time/TimeStamp.h) wraps POSIX struct timeval --
# genuine wall-clock time (seconds + microseconds since epoch), unlike the
# steady_clock correlation fields elsewhere. Carried as int64 microseconds
# since epoch.
#
# `tzoneInfo` (unsigned_8[20]) is a raw serialized timezone-info byte array,
# packed by ShmClock.h:65-87's set_TZ() overload -- carried as a fixed-size
# uint8[20] array verbatim, not decoded (decoding logic lives in
# tes_common_ais utility functions not present in this checkout).

uint32 shm_sec
int64 utc_sec_us
int32 utc_offset_min
uint8[20] tzone_info
