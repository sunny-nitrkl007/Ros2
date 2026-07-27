# Wire equivalent of LpsFloatIO_t (struct { LpsStat_t Stat; float Val; }).
# Structure inferred from serialize() call sites (LpsSaWeighTxChannel.h:471-472:
# `ar & LiftCylVel.Stat; ar & LiftCylVel.Val;`) -- the struct's own declaration
# is not in this checkout (part of the missing LpsPublic.h, see `stat` below).
# Used for LiftCylVel, LiftValveCommand, TiltValveCommand in LpsSaWeighTxChannel.msg.
#
# `stat` (LpsStat_t): BLOCKED -- LpsPublic.h, where this enum is actually
# defined, does not exist anywhere in this checkout (part of the missing
# lps_weighing submodule, Development-Plan.txt Step 0.2). Every use site in
# the tree references only the enumerator NAME (LPS_STATUS_BAD), never its
# numeric value -- unlike WeighBktWtAccuracy_t, no indirect evidence (mirror
# enum, ordering behavior) was found to reconstruct real values from.
# Carried as a raw int32 with no named constants for now: the shim's
# conversion function does static_cast<int32_t>(realEnumValue) against the
# real, still-compiled C++ enum, so the correct real value flows through
# regardless of whether the mapping is known here. Named constants get added
# once LpsPublic.h is sourced -- pure documentation addition, no behavior
# change.

int32 stat
float32 val
