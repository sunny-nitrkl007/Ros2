# Wire equivalent of LpsSaLftSealStatus_t (Legal For Trade seal status).
# Source: LpsSaWeighTxChannel.h:199-208
#
# `sealTime` here is std::chrono::system_clock::time_point -- NOT steady_clock
# like the other timePoint fields in this project. system_clock IS wall-clock
# time (epoch-based), so unlike the steady_clock fields (which are only valid
# for same-machine relative comparison, see LpsSaWeighTxChannel.msg), this one
# genuinely represents a calendar time and could reasonably be interpreted as
# such downstream. Still carried as a plain int64 nanosecond count rather than
# builtin_interfaces/Time for now, to keep the representation consistent with
# every other timestamp field in this project -- revisit if a consumer
# actually needs it as calendar time.

bool sealed
int64 seal_time_ns
uint32 seal_id
