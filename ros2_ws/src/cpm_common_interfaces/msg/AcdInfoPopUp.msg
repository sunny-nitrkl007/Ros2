# Wire equivalent of ACDInfoPopUp::type (std::bitset<15>).
# Source: LpsSaWeighTxChannel.h:42-64 -- bit positions fully verified
# (locally defined, sequential from 0, not blocked).
# std::bitset has no direct ROS2 equivalent; carried as uint32 with each
# real bit position named as a constant (the constant holds the bit INDEX,
# not a bitmask -- check with e.g. `(info_state >> PAYLOAD_NOT_ZEROED) & 1`).

uint8 PAYLOAD_TOO_HEAVY_TO_ZERO=0
uint8 PAYLOAD_NOT_ZEROED=1
uint8 PAYLOAD_CAL_ACCEPTED=2
uint8 PAYLOAD_ZERO_ACCEPTED=3
uint8 PAYLOAD_REWEIGH_LIFT_TOO_SLOW=4
uint8 PAYLOAD_REWEIGH_STOPPED_IN_RANGE=5
uint8 PAYLOAD_REWEIGH_NOT_RACKED=6
uint8 PAYLOAD_REWEIGH_SPEED_CHANGING=7
uint8 PAYLOAD_REWEIGH_INCONSISTENT=8
uint8 PAYLOAD_REWEIGH_PRESSURE_CHANGING=9
uint8 PAYLOAD_REWEIGH_EXCESSIVE_PITCH=10
uint8 PAYLOAD_REWEIGH_WARMUP_LIFT=11
uint8 PAYLOAD_RAISE_STALL=12
uint8 PAYLOAD_LOWER_STALL=13
uint8 PAYLOAD_CAL_WT_ENTRY_REQUIRED=14

uint32 bits
