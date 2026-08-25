#ifndef BMI_CDL_PID_H_
#define BMI_CDL_PID_H_

#ifdef __cplusplus
extern "C" {
#endif


#define BMI_CDL_DATA_STATUS_OK 0

constexpr uint32_t PAYLOAD_STORE_SWITCH_POSITION_PID                          = 0xF1AA;
constexpr uint32_t TRAVEL_LOADED_WHEEL_REVOLUTIONS_PID                        = 0xD026AF;
constexpr uint32_t PAYLOAD_CYCLE_CURRENT_DIG_TIME_PID                         = 0xD026B1;
constexpr uint32_t TIRE_ROLLING_RADIUS_PID                                    = 0xD00461;
constexpr uint32_t CPM_LVL2_FEATURE_INSTALL_STATUS_PID                        = 0xD11884;
constexpr uint32_t CPM_LVL2_FEATURE_TEMP_INSTALL_STATUS_PID                   = 0xD11887;
constexpr uint32_t CPM_FEATURE_ENABLE_STATUS_PID                              = 0xD10EE7;
constexpr uint32_t LOAD_HOLD_CHECK_VALVE_INSTALL_STATUS_PID                   = 0xD10F6E;
constexpr uint32_t DESIRED_GEAR_PID                                           = 0xF5D7;
constexpr uint32_t DIRECTION_SWITCH_POSITION                                  = 0xF074;
constexpr uint32_t TOTAL_OPERATING_HOURS                                      = 0xFC2D;
constexpr uint32_t STORE_BUTTON_PID                                           = 0xF1AA;
constexpr uint32_t TIPOFF_ASSSIST_PID                                         = 0xD118CE;
constexpr uint32_t MANUAL_ADD_PID                                             = 0xD11890;


#ifdef __cplusplus
}
#endif

#endif  // BMI_CDL_PID_H_
