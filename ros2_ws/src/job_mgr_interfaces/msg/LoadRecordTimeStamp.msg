# Wire equivalent of LpsSaLoadRecordTimeStamp.
# Source: LpsSaLoadRecordChannel/LpsSaLoadRecordSubtotal.h:19-45
#
# `utcTime` here is std::chrono::system_clock::time_point -- real wall-clock
# time (unlike the steady_clock correlation fields elsewhere in this
# project). Carried as int64 epoch nanoseconds, same representation choice
# as LftSealStatus.msg's seal_time_ns.

int64 utc_time_ns
uint32 shm_time
