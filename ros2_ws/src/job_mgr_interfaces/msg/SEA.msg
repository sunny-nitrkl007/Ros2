# Wire equivalent of SEA.
# Source: eta-ais/prod/machineCommon/content/prod/common/interfaces/SEAStatus/SEAStatus.h:124-186
# SEAStatusCodes values (SEAStatus.h:192-199) -- fully verified, explicit in
# source, not blocked.

uint16 STATUS_UNKNOWN=0
uint16 STATUS_NOT_INSTALLED=1
uint16 STATUS_INSTALLED=2
uint16 STATUS_ENABLED=3
uint16 STATUS_TEMP_INSTALLED=4

uint16 free_count
uint16 reason_code
uint16 status
