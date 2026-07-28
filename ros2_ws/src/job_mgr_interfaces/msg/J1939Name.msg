# Wire equivalent of j1939NameUtils::J1939_NAME_t.
# Source: eta-ais/prod/coretech/ais_util/ais/j1939NameUtils/j1939Name.h:26-55
# Field presence cross-checked against its own serialize() (lines 44-54) --
# exactly 7 fields, fully verified, not blocked.

uint8 industry_group
uint8 vehicle_system
uint8 vehicle_system_instance
uint8 function
uint8 function_instance
uint8 ecu_instance
uint16 manufacturer_code
