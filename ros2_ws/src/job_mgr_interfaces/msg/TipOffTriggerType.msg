# Wire equivalent of LpsSaTipOffTriggerType_t.
# Source: LpsSaJobMgrTxChannel.h:16-22, values from LpsPtPublic.h
#   TIP_OFF_TRIGGER_AUTO/MANUAL/DISABLED (lps_pass_tracker/include/LpsPtPublic.h:37-39)
# Shared by LpsSaJobMgrTxChannel.msg and LpsSaJobMgrReqstChannel.msg -- do not
# duplicate these constants locally in either message, reference this type.

uint8 AUTO=0
uint8 MANUAL=1
uint8 DISABLED=2

uint8 value
