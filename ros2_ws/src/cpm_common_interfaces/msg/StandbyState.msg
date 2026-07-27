# Wire equivalent of LpsSaJobMgrStandbyState_t.
# Source: CPM-Loader-ais/prod/common/interfaces/LpsSaJobMgrTxChannel/LpsSaJobMgrTxChannel.h:50-54
#   typedef enum {
#       LPS_SA_JOB_MGR_STANDBY_ACTIVATED = 0,
#       LPS_SA_JOB_MGR_STANDBY_DEACTIVATED
#   } LpsSaJobMgrStandbyState_t;
#
# Promoted here rather than left inline in job_mgr_interfaces because WeighApp
# uses this exact type directly, not just via a channel struct:
# CPM-Loader-ais/apps/LpsSaWeighApp/LpsSaWeighApp.h:256
#   LpsSaJobMgrStandbyState_t StandbyState = LPS_SA_JOB_MGR_STANDBY_DEACTIVATED;

uint8 ACTIVATED=0
uint8 DEACTIVATED=1

uint8 value
