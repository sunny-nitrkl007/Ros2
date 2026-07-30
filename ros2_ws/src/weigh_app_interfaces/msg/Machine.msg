################################################################################
# MachineStorage
################################################################################

uint16 app_number
float32 payload_ratio
uint16 payload_ratio_status
float32 payload_ratio_raw
uint16 payload_ratio_raw_status
float32 payload_weight
uint16 payload_weight_status
uint16 payload_calc_method

int16 lift_valve_cmd
int16 tilt_valve_cmd
int16 bucket_angle
float32 lift_angle
int16 tilt_lever_cmd
uint16 tilt_cyl_pos
uint16 impl_operation_status
uint16 machine_idle_status
uint8 dump_solenoid_percent
uint8 rack_solenoid_percent
uint8 lower_solenoid_percent
uint8 raise_solenoid_percent
float32 brake_pedal_position
uint16 lift_cylinder_he_pressure
uint16 lift_cylinder_re_pressure
int16 hydarulic_oil_temp
float32 bucket_payload

int16 desired_gear
int16 ground_speed
int16 engine_speed
int16 torque_converter_speed

uint16 keyswitch_state
uint32 real_time
uint8 real_time_status
int16 timezone_offset
uint32 total_fuel_high_res
uint32 total_fuel_low_res
uint16 fuel_rate
uint16 ard_fuel_rate
uint32 service_hour_meter

int16 external_force_normalized

uint32 gps_latitude
uint32 gps_longitude
uint8 gps_status
