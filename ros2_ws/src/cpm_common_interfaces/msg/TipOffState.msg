# Wire equivalent of LpsSaJobMgrTipOffState_t.
# Source: LpsSaJobMgrTxChannel.h:24-30, values from LpsPtPublic.h
#   TIP_OFF_MODE_PILE=0 / TIP_OFF_MODE_TRUCK=1 (lps_pass_tracker/include/LpsPtPublic.h:33-34)
# Shared by LpsSaJobMgrTxChannel.msg (tip_off_state, tip_off_state_cfg) and
# LpsSaJobMgrReqstChannel.msg (data.tipoffMode) -- do not duplicate locally.

uint8 PILE_ENABLE=0
uint8 TRUCK_ENABLE=1
uint8 UNAVAILABLE=2

uint8 value
