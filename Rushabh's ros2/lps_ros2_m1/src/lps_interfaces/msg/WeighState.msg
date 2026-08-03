# Compatibility subset consumed directly by the legacy Job Manager.
# Additional legacy fields will be added before bridge integration.
builtin_interfaces/Time stamp
uint64 source_monotonic_ns
uint32 sequence

uint16 dig_status
uint16 calibration_status
uint16 dump_status
float32 best_bucket_weight_tonnes
uint16 payload_calculation_method
bool bucket_weight_latched
bool latch_conditions_met
bool zero_available
float32 zero_weight
float32 simple_cal_adjust
lps_interfaces/LftSealStatus lft_seal_status
bool lift_stalled

bool payload_calibration_in_progress
uint8 test_status
bool excessive_pitch
bool bucket_fully_racked
