################################################################################
# DataLinkParamInfo::DlpParamIdentifierType_t
################################################################################

uint8 IDENTIFIER_TYPE_PID=0
uint8 IDENTIFIER_TYPE_PUBLIC_PGN=1
uint8 IDENTIFIER_TYPE_CAT_EXT=2
uint8 IDENTIFIER_TYPE_PROP_A_PGN=3
uint8 IDENTIFIER_TYPE_SPN=4

################################################################################
# DataLinkParamInfo::DlpUnits_t
################################################################################

uint8 UNITS_NO_UNIT=0
uint8 UNITS_KPA=1
uint8 UNITS_KPA_=2
uint8 UNITS_DEG_C=3
uint8 UNITS_KM_PER_HR=4
uint8 UNITS_RPM=5
uint8 UNITS_VOLTS=6
uint8 UNITS_LITERS=7
uint8 UNITS_PERCENT=8
uint8 UNITS_LTRS_PER_HR=9
uint8 UNITS_DEG=10
uint8 UNITS_MM=11
uint8 UNITS_MM_=12
uint8 UNITS_PULSES_PER_KM=13
uint8 UNITS_KW=14
uint8 UNITS_HRS=15
uint8 UNITS_SEC=16
uint8 UNITS_MS=17
uint8 UNITS_NM=18
uint8 UNITS_MIN=19
uint8 UNITS_RPM_PER_SEC=20
uint8 UNITS_METERS_PER_SEC=21
uint8 UNITS_HZ=22
uint8 UNITS_AMPS=23
uint8 UNITS_NM3_PER_HOUR=24
uint8 UNITS_MJ_PER_NM3=25
uint8 UNITS_OHMS=26
uint8 UNITS_METERS_PER_SEC2=27
uint8 UNITS_KG_PER_HR=28

################################################################################
# VarLengthDataLinkParamFactory::VarLengthParamType
################################################################################

uint8 VAR_LENGTH_PARAM_TYPE_NOT_APPLICABLE=0
uint8 VAR_LENGTH_PARAM_TYPE_GPS=1
uint8 VAR_LENGTH_PARAM_TYPE_SPL_PID_REV_DATA_BYTE_ORDER=2
uint8 VAR_LENGTH_PARAM_TYPE_TIME_ZONE=3
uint8 VAR_LENGTH_PARAM_TYPE_PRODUCT_ID=4
uint8 VAR_LENGTH_PARAM_TYPE_PGN=5
uint8 VAR_LENGTH_PARAM_TYPE_SPN=6
uint8 VAR_LENGTH_PARAM_TYPE_SECURITY_SYSTEM_CURRENT_KEY=7
uint8 VAR_LENGTH_PARAM_TYPE_SECURITY_SYSTEM_CURRENT_SECURITY_ID=8
uint8 VAR_LENGTH_PARAM_TYPE_UNSUPPORTED=9
uint8 VAR_LENGTH_PARAM_TYPE_CAT_EXT=10

################################################################################
# DataLinkParam
################################################################################

uint8 identifier_type
uint32 sid
uint32 param_id
uint8 units
float32 scaling
float32 offset

bool pid_data_received
uint16 last_value_dsi

int16 last_value_i16
uint16 last_value_u16
float32 last_value_f32
float64 last_value_eng
float64 last_good_value_eng

uint8[] last_value_vector
uint8[] var_param_block

uint8 var_length_param_type
uint16 var_length_param_dsi
