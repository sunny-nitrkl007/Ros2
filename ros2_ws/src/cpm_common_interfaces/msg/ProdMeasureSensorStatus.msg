# Wire equivalent of ProdMeasureSensorStatus_t (nested inside WeighPidData_t).
# Source: LpsSaWeighTxChannel.h:246-257
#
# tilt_sensor_config uses TiltSensorConfig_t -- defined locally in this same
# header (LpsSaWeighTxChannel.h:221-225), values fully verified, not blocked.

float32 lift_link_dc
float32 lift_cyl_pos
float32 lift_cyl_he_pres
float32 lift_cyl_re_pres

uint16 TILT_SENSOR_ROTARY_POSITION=451
uint16 TILT_SENSOR_IN_CYLINDER_POSITION=452
uint16 tilt_sensor_config

float32 tilt_cyl_he_pres
float32 tilt_cyl_re_pres
float32 tilt_link_dc
float32 hyd_oil_temp
