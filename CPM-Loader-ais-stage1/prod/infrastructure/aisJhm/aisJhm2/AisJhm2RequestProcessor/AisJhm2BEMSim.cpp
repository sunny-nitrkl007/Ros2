/*
 * AisJhm2BEMSim.cpp
 *
 *  Created on: Dec 23, 2021
 *      Author: pf
 */

#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"
#include "BEMSim/BEMSimCnfgMgr.h"

// BEM Simulator support
/******************************************************************************
FUNCTION: processParamRequestBEMSimInfo
DESCRIPTION:
    Populates the parameter request response message with the response.

    This request is called every 500ms from UI's BEM App page when BEM Simulator
    mode is enable. AisJhm server is the main process for the BEM Sim applet.

PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestBEMSimInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {

        { // charging (bool)
            const std::string& str = std::to_string(bemSimMgr.cnfg.chargeMode);
            m->addParameter(Jhm2SimpleParam("charging", str));
        }

        { // chargeLevel (float)
            const std::string& str = std::to_string(bemSimMgr.getChargeLevel());
            m->addParameter(Jhm2SimpleParam("chargeLevel", str));
        }

        { // chargeRate kW, > 0 for charging, present when charging is true
            const std::string& str = std::to_string(bemSimMgr.getChargeRate());
            m->addParameter(Jhm2SimpleParam("chargeRate", str));
        }

        { // dischargeRate kW, > 0 for discharging, present when charging is false
            float discharge_rate = bemSimMgr.getDischargeRate();
            if (discharge_rate > 0.f) {
                const std::string& str = std::to_string(discharge_rate);
                m->addParameter(Jhm2SimpleParam("dischargeRate", str));
            }
            else {
                const unsigned char dsi_cnm = 19; // Condition Not Met
                m->addParameter(Jhm2SimpleParam("dischargeRate", dsi_cnm, ""));
            }
        }

        { // float, minutes until discharged, present when charging is false
            float time_to_discharge = bemSimMgr.getTimeToDischarged();
            if ((time_to_discharge >= 0.f) && (time_to_discharge <= 9999.f)) {
                const std::string& str = std::to_string(time_to_discharge);
                m->addParameter(Jhm2SimpleParam("timeToDischarged", str));
            }
            else {
                const unsigned char dsi_cnm = 19; // Condition Not Met
                m->addParameter(Jhm2SimpleParam("timeToDischarged", dsi_cnm, ""));
            }
        }

        { // float, minutes until charged, present when charging is true
            const std::string& str = std::to_string(bemSimMgr.getTimeToCharged());
            m->addParameter(Jhm2SimpleParam("timeToCharged", str));
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}


/******************************************************************************
FUNCTION: processCmdRequestBEMSimCmdWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestBEMSimCmdWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaJobMgrReqstChannel msg;

    auto charge = pt.get_optional<bool>("charge");
    if (charge) {
        bemSimMgr.setChargeMode((bool)*charge);
        bemSimCnfgChanged_ = true;

    }

    return true;
}

/******************************************************************************
FUNCTION: processParamRequestbemSimCnfg
DESCRIPTION:Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:paramName - Requested Parameter Name, resp - Parameter Request Response Message
RETURN VALUE:true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestBEMSimCnfg(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        m->addParameter(Jhm2SimpleParam("enabled", bemSimMgr.cnfg.bemSimEnabled));
        m->addParameter(AisJhm2FloatArrayParam("chargerCapacities", bemSimMgr.getChargeCapacityList()));
        m->addParameter(Jhm2SimpleParam("chargerCapacity", bemSimMgr.cnfg.chargerCapacity));
        m->addParameter(Jhm2SimpleParam("engineDerateEnabled", bemSimMgr.cnfg.engineDerateEnabled));

        resp.insertParam(m);
        return true;
    }
    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestPayloadLevel2bemSimCnfgWr
DESCRIPTION: Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name, pt - Property tree containing data, resp - Command Request Response Message
RETURN VALUE: true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestBEMSimCnfgWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    // install status
    boost::optional<bool> enabled = pt.get_optional<bool>("enabled");
    boost::optional<float> chargerCapacity = pt.get_optional<float>("chargerCapacity");
    boost::optional<bool> engineDerateEnabled = pt.get_optional<bool>("engineDerateEnabled");
    boost::optional<float> stateOfCharge = pt.get_optional<float>("stateOfCharge");

    if (enabled) {
        bemSimMgr.setBEMSimEnable((bool)*enabled);
        bemSimCnfgChanged_ = true;
    }

    if (chargerCapacity) {
        bemSimMgr.setChargerCapacity((float)*chargerCapacity);
        bemSimCnfgChanged_ = true;

    }

    if (engineDerateEnabled) {
        bemSimMgr.cnfg.engineDerateEnabled = (bool)*engineDerateEnabled;
        bemSimCnfgChanged_ = true;

    }

    if (stateOfCharge) {
        float soc = (float)*stateOfCharge;
        if(( soc >= 0.0f) && (soc <= 100.0f)) {
            bemSimMgr.setChargeLevel((float)*stateOfCharge);
            bemSimCnfgChanged_ = true;
        }
    }

    return true;
}
