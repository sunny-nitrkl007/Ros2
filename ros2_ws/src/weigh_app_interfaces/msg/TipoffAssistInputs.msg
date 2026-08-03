################################################################################
# TipoffAssistInputs
################################################################################

# Input status bitfield packed into uint16
uint16 input_status

bool invalidate_outputs

float32 lift_valve_cmd
float32 tilt_valve_cmd
float32 tilt_extension
float32 lift_angle

float32 lift_he_pressure
float32 lift_re_pressure
float32 tilt_he_pressure
float32 tilt_re_pressure

float32 eef_imu_accel_x
float32 eef_imu_accel_y
float32 eef_imu_accel_z

float32 steering_angle
float32 tool_mass

float32 truck_target_wt
float32 truck_start_weight

uint8 bucket_current_weight_accuracy
float32 bucket_current_weight

uint8 tipoff_mode

float32 zero_offset
float32 simple_cal_factor
float32 mach_pitch_cal_offset

bool unlatch_trigger

float32 bucket_angle

float32 anchor_zero_offset
float32 anchor_factor

uint16 pass_count

float32 lift_norm_angle
float32 lift_norm_length

float32 tilt_norm_angle
float32 tilt_norm_length

float32 friction_mu
float32 friction_offset
