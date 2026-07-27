# Wire equivalent of LpsSaPayload_t.
# Source: LpsSaWeighTxChannel.h:188-197
# `payload_ratio_status` (LpsSaPayloadRatioStatus_t = LpsInstWtRawStat_t):
# BLOCKED, same situation as FloatIO.msg's `stat` -- raw int32, no named
# constants yet. Real enumerator names seen in source (not values):
# LPS_INST_RAW_WT_STATUS_OK / _BAD / _RAISE_STALL / _LOWER_STALL
# (LpsSaWeighTxChannel.h:183-186).

float32 payload_ratio_raw
float32 payload_ratio
int32 payload_ratio_status
