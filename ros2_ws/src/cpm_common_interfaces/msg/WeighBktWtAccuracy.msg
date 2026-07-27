# Wire equivalent of LpsWeighBktWtAccuracy_t.
#
# PROVISIONAL: the real typedef lives in LpsCommonStructures.h, part of the
# lps_common submodule, which is NOT present in this checkout (Step 0 of
# Development-Plan.txt). Values below are reconstructed from indirect
# evidence, not read from the real header:
#   - LpsSaTotalsApp/CPMTicket/CPMTicket.pb.h: a protobuf mirror enum for the
#     same concept -- Ticket_Measurement_ACCURACY_{NONE=0,LOW=1,MEDIUM=2,HIGH=3}
#   - LpsSaLoadRecordSubtotal.h:316-330: scans passes starting from HIGH and
#     downgrades via `if (pa < a) a = pa`, consistent with ascending-quality
#     ordering (NONE lowest, HIGH highest)
#   - LpsSaLoadRecordSubtotal.h:333-334: value is shifted into an 8-bit field
#     and masked to 4 bits elsewhere, consistent with a small 0-3 range
# CONFIRM against the real header once lps_common is sourced.

uint8 NONE=0
uint8 LOW=1
uint8 MED=2
uint8 HIGH=3

uint8 value
