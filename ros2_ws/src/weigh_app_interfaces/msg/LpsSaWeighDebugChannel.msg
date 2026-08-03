################################################################################
# LpsSaWeighDebugChannelStorage
################################################################################

uint16 reweigh_warn

int32 debug_lift_cyl_he_period
int32 debug_lift_cyl_he_width
float32 debug_lift_cyl_he_dc
float32 debug_lift_cyl_he_press
int32 debug_lift_cyl_he_press_status

int32 debug_lift_cyl_re_period
int32 debug_lift_cyl_re_width
float32 debug_lift_cyl_re_dc
float32 debug_lift_cyl_re_press
int32 debug_lift_cyl_re_press_status

int32 debug_tilt_cyl_he_period
int32 debug_tilt_cyl_he_width
float32 debug_tilt_cyl_he_dc
float32 debug_tilt_cyl_he_press
int32 debug_tilt_cyl_he_press_status

int32 debug_tilt_cyl_re_period
int32 debug_tilt_cyl_re_width
float32 debug_tilt_cyl_re_dc
float32 debug_tilt_cyl_re_press
int32 debug_tilt_cyl_re_press_status

int32 debug_lift_cyl_period
int32 debug_lift_cyl_width
float32 debug_lift_cyl_dc
float32 debug_lift_cyl_raw_ext
float32 debug_lift_cyl_filt_ext
float32 debug_lift_cyl_norm_len
float32 debug_lift_cyl_vel
float32 debug_lift_angle
float32 debug_lift_ang_vel
int32 debug_lift_position_status

int32 debug_tilt_cyl_period
int32 debug_tilt_cyl_width
float32 debug_tilt_cyl_dc
float32 debug_tilt_cyl_raw_len
float32 debug_tilt_cyl_norm_len
float32 debug_tilt_cyl_vel
float32 debug_tilt_angle
float32 debug_tilt_angle_abc
int32 debug_tilt_position_status

float32 debug_bucket_angle
int32 debug_bucket_angle_status
float32 debug_hyd_oil_temp
int32 debug_hyd_oil_temp_status
uint16 debug_requested_gear
int32 debug_requested_gear_status
float32 debug_weigh_updt_loader_bkt_payld_trgt_wt
uint32 debug_weigh_updt_clock_keyon_sec
uint32 debug_weigh_updt_pass_count

float32 best_bkt_wt_payload_calc_meth
float32 best_bkt_wt_wt
bool best_bkt_wt_tipoff_live_weight_in_use

float32 cyl_flow_const_lift_he_flow_const
float32 cyl_flow_const_lift_re_to_he_flow_ratio

bool dig_detect_data_dig_detected
float32 dig_detect_data_dig_duration
bool dig_detect_data_dig_started
uint8 dig_detect_data_dig_state
bool dig_detect_data_wt_stabilized

float32 dump_wt_change_data_dump_cumulative_wt_change

float32 inst_wt_der
float32 inst_wt_filt
float32 inst_wt_llw_filt
cpm_common_interfaces/FloatIO inst_wt_raw
float32 inst_wt_raw_pre_tilt_comp
float32 inst_wt_raw_post_tilt_comp

float32 lift_cyl_pressure
float32 lift_cyl_pressure_v0
float32 lift_cyl_pressure_imu_adjusted

uint8 live_weigh_tbl_stat
float32 live_weigh_tbl_wt
bool live_weigh_tbl_use_fast_filter

float32 low_lift_wt_wt
float32 low_lift_wt_confidence
float32 low_lift_wt_confidence_est
float32 low_lift_wt_confidence_timer
float32 low_lift_wt_last_input
float32 low_lift_wt_mean_est
float32 low_lift_wt_mean_est_comp
uint8 low_lift_wt_stat
float32 low_lift_wt_stdev_est
float32 low_lift_wt_variance_est

uint8 dump_state_status
bool lps_stall_detect_lift_stalled

float32 linkage_movement_lift
float32 linkage_movement_tilt

float32 wrw_tbl_wk_tbl_result_weight_ave_raw
float32 wrw_tbl_wk_tbl_result_weight_ave
float32 wrw_tbl_op_tbl_wt
bool wrw_tbl_op_tbl_warning
int32 wrw_tbl_op_tbl_indicator
uint8 wrw_tbl_op_tbl_status
float32 wrw_tbl_wk_tbl_result_weight_ave_raw_imu_comp

bool wrw_tbl_wk_tbl_been_below_range
uint8 zero_adj_tbl_zero_adj_status
float32 overload_warn_tbl_bucket_load_factor
bool overload_warn_tbl_dig_reverse_flag
bool overload_warn_tbl_in_overload_range
uint8 overload_warn_tbl_level
uint8 overload_warn_tbl_prev_dig_state

float32 lift_position_filt_length
float32 lift_position_filt_velocity
float32 lift_position_filt_angle

TipoffAssistInputs tipoff_inputs
TipoffAssistOutputs tipoff_outputs

float32 chassis_imu_gravity_x
float32 chassis_imu_gravity_y
float32 chassis_imu_gravity_z
float32 chassis_imu_velocity_x
float32 chassis_imu_velocity_y
float32 chassis_imu_velocity_z
float32 chassis_imu_bias_mag_jerk
float32 chassis_imu_bias_mag_ang_accel
float32 chassis_imu_bias_max_ang_vel
float32 chassis_imu_bias_ang_vel_x
float32 chassis_imu_bias_ang_vel_y
float32 chassis_imu_bias_ang_vel_z
float32 chassis_imu_pitch
float32 chassis_imu_roll
uint8 chassis_imu_sensor_lin_acc_status
uint8 chassis_imu_sensor_ang_vel_status
float32 chassis_imu_lin_acc_x
float32 chassis_imu_lin_acc_y
float32 chassis_imu_lin_acc_z
float32 chassis_imu_ang_vel_x
float32 chassis_imu_ang_vel_y
float32 chassis_imu_ang_vel_z
