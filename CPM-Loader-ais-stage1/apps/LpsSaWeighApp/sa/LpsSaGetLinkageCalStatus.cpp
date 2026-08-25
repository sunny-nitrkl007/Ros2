/******************************Standalone********************************/
#include "../LpsSaWeighApp.h"

bool LpsSaWeighApp::GetLiftCylCalStatus(void) {
    return (0 != (liftCalNvmTbl_.lift_cal_stat & CAL_LIFT_LINKAGE_MASK));
}

bool LpsSaWeighApp::GetTiltCylCalStatus(void) {
    return (0 != (tiltCalNvmTbl_.tilt_cal_stat & CAL_TILT_LINKAGE_MASK));
}
