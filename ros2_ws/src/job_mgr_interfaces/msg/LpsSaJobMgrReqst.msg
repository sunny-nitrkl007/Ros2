# Wire equivalent of LpsSaJobMgrReqst.
# Source: LpsSaJobMgrReqstChannel.h:18-84
#
# The original class exposes arg1-4 through named accessors depending on
# `command` (materialId()/materialName()/truckId()/tagValue() etc all just
# read/write the same 4 underlying fields) -- that's a C++ convenience layer,
# not separate wire data. The wire format carries the 4 raw fields directly;
# the shim's conversion function is where the accessor-style interpretation
# happens, same as it does in the original code today.

# LpsSaJobMgrReqstCommand (LpsSaJobMgrReqstChannel.h:18-30) -- values verified,
# plain sequential enum class, no explicit values in source so 0..10 in
# declaration order.
uint8 WRITE_MATERIAL_ID=0
uint8 WRITE_MATERIAL_NAME=1
uint8 WRITE_MATERIAL_DENSITY=2
uint8 WRITE_TRUCK_ID=3
uint8 WRITE_TRUCK_NAME=4
uint8 WRITE_TRUCK_TARGET_WEIGHT=5
uint8 WRITE_TAG1=6
uint8 WRITE_TAG2=7
uint8 WRITE_TAG3=8
uint8 WRITE_TAG4=9
uint8 NONE=10
uint8 command

string arg1
float32 arg2
uint32 arg3
string arg4
