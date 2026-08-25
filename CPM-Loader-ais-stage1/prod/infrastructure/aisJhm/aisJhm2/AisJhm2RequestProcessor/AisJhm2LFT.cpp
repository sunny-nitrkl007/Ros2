/*
 * AisJhm2LFT.cpp
 */
#include <chrono/print.hpp>
#include <chrono/tz.hpp>

#include <release/legal_for_trade_identifiers.h>

#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"
#include "AisJhm2UintParam.h"
#include "AisJhm2BoolParam.h"

void AisJhm2RequestProcessor::installLFTHandlers() {
    paramMsgRequestHandlerMap_["LFTSummary"] = &AisJhm2RequestProcessor::processParamRequestLFTSummary;
    paramMsgRequestHandlerMap_["LFTSealed"] = &AisJhm2RequestProcessor::processParamRequestLFTSealed;
    paramMsgRequestHandlerMap_["LFTComponentIds"] = &AisJhm2RequestProcessor::processParamRequestLFTComponentIds;
    paramMsgRequestHandlerMap_["LFTSettings"] = &AisJhm2RequestProcessor::processParamRequestLFTSettings;
    cmdMsgRequestHandlerMap_["LFTSealed_Wr"] = &AisJhm2RequestProcessor::processCmdRequestLFTSealedWr;
    cmdMsgRequestHandlerMap_["LFTSensorIds_Wr"] = &AisJhm2RequestProcessor::processCmdRequestLFTSensorIdsWr;
    cmdMsgRequestHandlerMap_["LFTSettings_Wr"] = &AisJhm2RequestProcessor::processCmdRequestLFTSettingsWr;
    cmdMsgRequestHandlerMap_["LFTEnable_Wr"] = &AisJhm2RequestProcessor::processCmdRequestLFTEnableWr;
}

bool AisJhm2RequestProcessor::processParamRequestLFTSummary(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam("LFTSummary");
    if (nullptr != m) {
        // Payload ECM Equipment ID
        m->addParameter(Jhm2SimpleParam("equipmentId", partNumbers_.GetEquipmentId()));

        // Legal For Trade Software Identifier
        m->addParameter(Jhm2SimpleParam("softwareVersion", legal_for_trade_software_id_version + "." + legal_for_trade_software_id_build));

        // Payload System Model and Type Approval Number (string)
        if (legalForTradeInstalled_) {
            m->addParameter(Jhm2SimpleParam("model", "N8N"));
            m->addParameter(Jhm2SimpleParam("typeApprovalNumber", LEGAL_FOR_TRADE_TYPE_APPROVAL_NUMBER));
            if (weighAppTxData_.LftSealStatus.sealed) {
                m->addParameter(AisJhm2BoolParam("sealed", true));
                m->addParameter(AisJhm2UintParam("sealIdentifier", weighAppTxData_.LftSealStatus.sealId));

                // Sealed Date/Time
                if (weighAppTxData_.LftSealStatus.sealTime > std::chrono::system_clock::time_point::min()) {
                    std::string timeStr = tes_common_ais::putLocalTime(weighAppTxData_.LftSealStatus.sealTime, "%F");
                    m->addParameter(Jhm2SimpleParam("dateSealed", timeStr));
                }
                else {
                    m->addParameter(Jhm2SimpleParam("dateSealed", 19 /* DSI - Conditions Not Met */, "***"));
                }

                m->addParameter(Jhm2SimpleParam("dateUnsealed", 16 /* DSI - Parameter Not Available */, ""));
            }
            else {
                m->addParameter(AisJhm2BoolParam("sealed", false));
                m->addParameter(AisJhm2UintParam("sealIdentifier", 16 /* DSI - Parameter Not Available */, 0));

                m->addParameter(Jhm2SimpleParam("dateSealed", 16 /* DSI - Parameter Not Available */, ""));

                // Unsealed Date/Time
                if (weighAppTxData_.LftSealStatus.sealTime > std::chrono::system_clock::time_point::min()) {
                    std::string timeStr = tes_common_ais::putLocalTime(weighAppTxData_.LftSealStatus.sealTime, "%F");
                    m->addParameter(Jhm2SimpleParam("dateUnsealed", timeStr));
                }
                else {
                    m->addParameter(Jhm2SimpleParam("dateUnsealed", 19 /* DSI - Conditions Not Met */, "***"));
                }
            }
        }
        else {
            m->addParameter(Jhm2SimpleParam("model", "C55"));
            m->addParameter(Jhm2SimpleParam("typeApprovalNumber", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(AisJhm2BoolParam("sealed", 20 /* DSI - Disabled or Not Installed */, false));
            m->addParameter(AisJhm2UintParam("sealIdentifier", 20 /* DSI - Disabled or Not Installed */, 0));
            m->addParameter(Jhm2SimpleParam("dateSealed", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("dateUnsealed", 20 /* DSI - Disabled or Not Installed */, ""));
        }

        resp.insertParam(m);

        return true;
    }
    else {
        AIS_LOG_ERROR("LFTSummary Parameter Request unsuccessful");
        return false;
    }
}

bool AisJhm2RequestProcessor::processParamRequestLFTSealed(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2BoolParam* p = nullptr;

    if (legalForTradeInstalled_) {
        p = new AisJhm2BoolParam("LFTSealed", weighAppTxData_.LftSealStatus.sealed);
    }
    else {
        p = new AisJhm2BoolParam("LFTSealed", 20 /* DSI - Disabled or Not Installed */, false);
    }

    if (nullptr != p) {
        resp.insertParam(p);
        return true;
    }

    AIS_LOG_ERROR("LFTSealed Parameter Request unsuccessful");
    return false;
}

bool AisJhm2RequestProcessor::processParamRequestLFTComponentIds(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam("LFTComponentIds");
    if (nullptr != m) {
        if (legalForTradeInstalled_) {
            m->addParameter(Jhm2SimpleParam("liftPositionSensorId", weighAppTxData_.LiftPositionSensorId));
            m->addParameter(Jhm2SimpleParam("tiltPositionSensorId", weighAppTxData_.TiltPositionSensorId));
            m->addParameter(Jhm2SimpleParam("liftHeadEndPressureSensorId", weighAppTxData_.LiftHeadEndPressureSensorId));
            m->addParameter(Jhm2SimpleParam("liftRodEndPressureSensorId", weighAppTxData_.LiftRodEndPressureSensorId));
            m->addParameter(Jhm2SimpleParam("hydraulicOilTemperatureSensorId", weighAppTxData_.HydraulicOilTemperatureSensorId));
            m->addParameter(Jhm2SimpleParam("inertialMeasurementSensorId", weighAppTxData_.ImuSensorId));
            m->addParameter(Jhm2SimpleParam("analysisModuleSN", partNumbers_.GetECMSerialNum()));
            m->addParameter(Jhm2SimpleParam("implementControlSN", weighAppTxData_.ImplementSerialNum));
        }
        else {
            m->addParameter(Jhm2SimpleParam("liftPositionSensorId", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("tiltPositionSensorId", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("liftHeadEndPressureSensorId", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("liftRodEndPressureSensorId", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("hydraulicOilTemperatureSensorId", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("inertialMeasurementSensorId", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("analysisModuleSN", 20 /* DSI - Disabled or Not Installed */, ""));
            m->addParameter(Jhm2SimpleParam("implementControlSN", 20 /* DSI - Disabled or Not Installed */, ""));
        }

        resp.insertParam(m);

        return true;
    }
    else {
        AIS_LOG_ERROR("LFTComponentIds Parameter Request unsuccessful");
        return false;
    }
}

bool AisJhm2RequestProcessor::processParamRequestLFTSettings(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam("LFTSettings");
    if (nullptr != m) {
        m->addParameter(AisJhm2BoolParam("verificationModeEnabled", displayState_.isInVerificationMode()));

        resp.insertParam(m);

        return true;
    }
    else {
        AIS_LOG_ERROR("LFTSettings Parameter Request unsuccessful");
        return false;
    }
}

bool AisJhm2RequestProcessor::processCmdRequestLFTSealedWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "LFTSealed_Wr") {
        return false;
    }

    // Must be in service mode
    if (!displayState_.isInServiceMode()) {
        return false;
    }

    auto sealed = pt.get_value_optional<bool>();
    if (sealed) {
        LpsSaWeighReqstChannel req;
        req.command = LpsSaWeighReqstChannel::Command::WRITE_LFT_SEALED;
        req.arg.b = *sealed;
        weighAppInf_.sendRequestWaitForTxData(req, weighAppTxData_);
        return true;
    }

    return false;
}

bool AisJhm2RequestProcessor::processCmdRequestLFTSensorIdsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "LFTSensorIds_Wr") {
        return false;
    }

    // Must be in service mode
    if (!displayState_.isInServiceMode()) {
        return false;
    }

    bool weighAppReqMade = false;

    if (!legalForTradeInstalled_ || !weighAppTxData_.LftSealStatus.sealed) {
        { // liftPositionSensorId
            auto sensorId = pt.get_optional<std::string>("liftPositionSensorId");
            if (sensorId) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_LIFT_POSITION_SENSOR_ID;
                req.arg.s = (*sensorId).substr(0, 20);
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

        { // tiltPositionSensorId
            auto sensorId = pt.get_optional<std::string>("tiltPositionSensorId");
            if (sensorId) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_TILT_POSITION_SENSOR_ID;
                req.arg.s = (*sensorId).substr(0, 20);
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

        { // liftHeadEndPressureSensorId
            auto sensorId = pt.get_optional<std::string>("liftHeadEndPressureSensorId");
            if (sensorId) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_LIFT_HE_PRESSURE_SENSOR_ID;
                req.arg.s = (*sensorId).substr(0, 20);
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

        { // liftRodEndPressureSensorId
            auto sensorId = pt.get_optional<std::string>("liftRodEndPressureSensorId");
            if (sensorId) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_LIFT_RE_PRESSURE_SENSOR_ID;
                req.arg.s = (*sensorId).substr(0, 20);
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

        { // hydraulicOilTemperatureSensorId
            auto sensorId = pt.get_optional<std::string>("hydraulicOilTemperatureSensorId");
            if (sensorId) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_HYDRAULIC_OIL_TEMP_SENSOR_ID;
                req.arg.s = (*sensorId).substr(0, 20);
                weighAppInf_.sendRequest(req);
                weighAppReqMade = true;
            }
        }

    }

    if (weighAppReqMade) {
        weighAppInf_.waitForTxData(weighAppTxData_);
    }

    return true;
}

bool AisJhm2RequestProcessor::processCmdRequestLFTSettingsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "LFTSettings_Wr") {
        return false;
    }

    { // verificationModeEnabled
        auto enabled = pt.get_optional<bool>("verificationModeEnabled");
        if (enabled) {
            displayState_.setVerificationMode(*enabled);
        }
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestLFTEnableWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestLFTEnableWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("LFTEnable_Wr" == cmd) {
        // Set LFT Enable
        boost::optional<bool> enable = pt.get_value_optional<bool>();
        if (enable) {
            LpsSaJobMgrReqstChannel req;
            req.command = LpsSaJobMgrReqstChannel::Command::WRITE_LFT_DISABLED;
            req.data.enabled = !(*enable);
            jobMgrHelper_.sendRequest(req);
            return true;
        }
    }

    return false;
}

