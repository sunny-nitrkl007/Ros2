/******************************Advanced********************************/
#include "../AutonomyConditionDiagnostics.h"
#include "autonomyConditions/conditions/DL_ETHERNET_1_CID_3900_FMI_9.h"
#include "autonomyConditions/conditions/DL_ETHERNET_1_CID_3900_FMI_14.h"

void AutonomyConditionDiagnostics::CheckEthernetDiags(void)
{
    auto steadyNow = std::chrono::steady_clock::now();

    /*
     * We expect the display heartbeat message to keep coming every 5 seconds.
     * If we don't get it for 20 seconds, then we have a problem.  Of course
     * the diagnostic is inhibited around power cycles and software updates (flashes)
     */
    if (displayHeartbeatLastCount_ != displayState_.getHeartbeatCount()) {
        displayHeartbeatLastCount_ = displayState_.getHeartbeatCount();
        displayHeartbeatLastTime_ = steadyNow;
    }

    if (steadyNow > (displayHeartbeatLastTime_ + std::chrono::seconds(20))) {
        setAutonomyCondition(DL_ETHERNET_1_CID_3900_FMI_9());
    }
    else {
        clearAutonomyCondition<DL_ETHERNET_1_CID_3900_FMI_9>();
    }

    /*
     * If we cannot get the product link ecm summary, then log a diagnostic
     */
    if (ecmSummaryProductLink_.isConnectionBad()) {
        setAutonomyCondition(DL_ETHERNET_1_CID_3900_FMI_14());
    }
    else {
        clearAutonomyCondition<DL_ETHERNET_1_CID_3900_FMI_14>();
    }
}
