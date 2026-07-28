# Wire equivalent of Diagnostic.
# Source: eta-ais/prod/machineCommon/content/prod/common/diagnostic/Diagnostic.h:22-562
# Field presence cross-checked against serialize() case 6 (lines 526-548,
# current version -- BOOST_CLASS_VERSION(Diagnostic, 6) at line 564).
#
# gpsTime_sec (TimeStamp) -- same wall-clock representation as
# ShmClockInput.msg's utc_sec_us / LoadRecordTimeStamp.msg's utc_time_ns.
#
# last_occ_duration/pid/worst_case_val/event_trig_val are only meaningfully
# populated in the original when last_occurrence_set is true (conditional
# archiving in source) -- carried here unconditionally since .msg has no
# conditional fields; treat them as meaningless when last_occurrence_set is
# false, same as the original semantics.

uint16 id
uint16 cid
uint8 fmi
uint8 wci
uint8 annunc_value
string description
uint16 subsystem
int64 gps_time_sec_us
uint32 service_hour_meter_sec
uint16 occurrence_count
uint8 category
uint8 status
bool last_occurrence_set
uint16 last_occ_duration
uint32 pid
uint32 worst_case_val
uint32 event_trig_val
J1939Name jname
