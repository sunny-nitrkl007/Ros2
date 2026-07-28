# Wire equivalent of EventDiagnosticDataStorage (consumed as
# EventDiagnosticDataInput, live call site LpsSaJobMgrScs.cpp:688 ->get()).
# Source: eta-ais/prod/machineCommon/content/prod/common/interfaces/EventDiagnosticData/EventDiagnosticData.h
# (same machineCommon location as ShmClockInput.msg -- not under
# CPM-Loader-ais/prod/common/interfaces/ like the LpsSa*/AisJhm2* channels.)
#
# Field presence cross-checked against serialize() case 4 (lines 82-91,
# current version -- BOOST_CLASS_VERSION(EventDiagnosticDataStorage, 4)).
# `dnsname` kept as one word to match the real member name exactly.

string dnsname
Diagnostic[] active_diagnostics
uint32 payload_crc
int64 payload_last_update_time_us
uint8 highest_warning_level
uint8 highest_annunciation_level
string source
