# Wire equivalent of LpsSaLiftPosition_t.
# Source: LpsSaWeighTxChannel.h:146-161
# `status` (LpsStat_t): same BLOCKED situation as FloatIO.msg's `stat` --
# raw int32, no named constants yet, see that file for the full explanation.

float32 angle
float32 percent_angle
float32 cylinder_length
float32 percent_cylinder_length
float32 cylinder_extension
int32 status
