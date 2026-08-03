################################################################################
# CalRespCode_t
################################################################################

uint8 RESP_CAL_SUCCESS=0
uint8 RESP_CAL_FAIL=1
uint8 RESP_CAL_UNKNOWN=2

################################################################################
# CalMgrCmdResp::CalibrationResp
################################################################################

uint8 resp_code
uint8 cal_resp
uint8 step_no
uint16 error
uint16 warning
bool enable_qual_read
