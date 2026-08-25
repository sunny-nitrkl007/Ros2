/*
 * AisJhm2ServiceHistory.cpp
 *
 *  Created on: May 19, 2023
 *      Author: pf
 */
#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighReqstChannel.h>

#include <jhm2/Jhm2Param/Jhm2SimpleParam.h>

#include "AisJhm2RequestProcessor.h"
#include "AisJhm2FloatParam.h"
#include "AisJhm2MultiParam.h"


/******************************************************************************
FUNCTION: processParamRequestMachImuPitchOffset
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestMachImuPitchOffset(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp)
{
    /*
     * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
     * to strings, so if we want control over precision, we have to convert it to a string here before
     * it gets its grimy hands on it.
     */
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << weighAppTxData_.PidData.TipoffPitchCalOffset;
    const std::string& str = ss.str();

    Jhm2SimpleParam* p = new Jhm2SimpleParam("MachImuPitchOffset", str);
    if (nullptr != p) {
        resp.insertParam(p);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestTipoffWeightAdjustData
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestTipoffWeightAdjustData(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp)
{
    /*
     * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
     * to strings, so if we want control over precision, we have to convert it to a string here before
     * it gets its grimy hands on it.
     */

    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);

    if (nullptr != m) {
        // Units
        m->addParameter(Jhm2SimpleParam("units", weightUnitsString()));

        { // Offset
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << lps_common_weight_from_tonnes(weighAppTxData_.ToaAnchoredZeroOffset, displayState_.getSettings().weightUnits);
            m->addParameter(Jhm2SimpleParam("offset", ss.str()));
        }

        { // Slope
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << weighAppTxData_.ToaAnchoredFactor;
            m->addParameter(Jhm2SimpleParam("slope", ss.str()));
        }

        // Status
        std::string status;
        switch (weighAppTxData_.ToaAnchorStatus) {
        case LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::UNCHANGED:
            status = "UNCHANGED";
            break;
        case LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::ACCEPTED:
            status = "ACCEPTED";
            break;
        case LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::REJECTED:
            status = "REJECTED";
            break;
        case LpsSaWeighTxChannelStorage::ToaAnchorStatus_t::RESET:
            status = "RESET";
            break;
        default:
            status = "?";
            break;
        };

        m->addParameter(Jhm2SimpleParam("status", status));

        /* insert params to resp */
        resp.insertParam(m);

        return true;
    }

    /* something went wrong */
    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestMachImuPitchOffset
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestMachImuPitchOffset(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp)
{
    auto oOffset = pt.get_value_optional<float>();
    if (oOffset) {
        float offset = *oOffset;
        LpsSaWeighReqstChannel req;
        req.command = LpsSaWeighReqstChannel::Command::WRITE_MACHINE_PITCH_CAL_OFFSET;
        req.arg.f1 = offset;
        weighAppInf_.sendRequestWaitForTxData(req, weighAppTxData_);
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestTipassistWeightData
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTipoffWeightAdjustData(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp)
{
    if ("TipoffWeightAdjustData_Wr" == cmd) {

        AisJhm2TxChannel req;

        auto value = pt.get_optional<float>("TipoffWeight1");
        if (value) {
            req.tipoff_weight_adjust_data.tipOffWeight1 = weightUnitConvertFromDisplay(*value);
            req.tipoff_weight_adjust_data.newDataFlag = true;
        }

        value = pt.get_optional<float>("WeighRangeWeight1");
        if (value) {
            req.tipoff_weight_adjust_data.weighRangeWeight1 = weightUnitConvertFromDisplay(*value);
            req.tipoff_weight_adjust_data.newDataFlag = true;
        }

        value = pt.get_optional<float>("TipoffWeight2");
        if (value) {
            req.tipoff_weight_adjust_data.tipOffWeight2 = weightUnitConvertFromDisplay(*value);
            req.tipoff_weight_adjust_data.newDataFlag = true;
        }

        value = pt.get_optional<float>("WeighRangeWeight2");
        if (value) {
            req.tipoff_weight_adjust_data.weighRangeWeight2 = weightUnitConvertFromDisplay(*value);
            req.tipoff_weight_adjust_data.newDataFlag = true;
        }

        /* Publish if we have new data to send */
        if (req.tipoff_weight_adjust_data.newDataFlag &&
                (nullptr != aisJhm2TxOutput_)) {
            aisJhm2TxOutput_->publish(req);
        }
    }

    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestTipassistWeightDataReset
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTipoffWeightAdjustReset(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp)
{
    auto oArg = pt.get_value_optional<bool>();
    if (oArg && *oArg) {

        AisJhm2TxChannel req;

        /* set reset flag and send */
        req.tipoff_weight_adjust_data.reset = true;

        if (nullptr != aisJhm2TxOutput_) {
            aisJhm2TxOutput_->publish(req);
        }
    }

    return true;
}

/*
 * Function: processParamRequestIMUData
 * Purpose: Provide live IMU data for the service dashboard.
 */
bool AisJhm2RequestProcessor::processParamRequestIMUData(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp)
{
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);

    if (nullptr != m) {
        // float nan_val = std::numeric_limits<double>::quiet_NaN();
        // m->addParameter(AisJhm2FloatParam("machinePitch", nan_val));
        m->addParameter(AisJhm2FloatParam("machinePitch", (float)weighAppTxData_.PidData.MachinePitch / 10.f));
        m->addParameter(AisJhm2FloatParam("machineRoll", (float)weighAppTxData_.PidData.MachineRoll / 100.f));
        resp.insertParam(m);
        return true;
    }

    return false;
}

/*
 * Function: processCmdRequestPayloadServiceDashboard
 * Purpose: Handle write requests for the service dashboard.
 */
bool AisJhm2RequestProcessor::processCmdRequestPayloadServiceDashboard(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp)
{
    if ("WeighCalibration_Wr" == cmd) {
        // Not enforcing service mode here since they are already on the service dashboard, which
        // is a user interface already entirely dedicated to service.
        if (!(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            { // advCalAdjust
                boost::optional<const boost::property_tree::ptree&> advCalAdjust = pt.get_child_optional("advCalAdjust");
                if (advCalAdjust) {
                    LpsSaWeighReqstChannel req;
                    req.command = LpsSaWeighReqstChannel::Command::WRITE_ADVANCED_CALIBRATION_ADJUSTMENT;
                    for (auto& point : *advCalAdjust) {
                        if (2 == point.second.size()) {
                            auto x = point.second.front().second.get_value_optional<float>();
                            auto y = point.second.back().second.get_value_optional<float>();
                            if (x && y) {
                                req.arg.map.emplace_back(*x, *y);
                            }
                        }
                    }
                    weighAppInf_.sendRequest(req);
                }
            }
        }

        return true;
    }
    else if ("WeighConfiguration_Wr" == cmd) {
        // Not enforcing service mode here since they are already on the service dashboard, which
        // is a user interface already entirely dedicated to service.
        if (!(legalForTradeInstalled_ && weighAppTxData_.LftSealStatus.sealed)) {
            { // reweighMaxPitch
                auto reweighMaxPitch = pt.get_optional<float>("reweighMaxPitch");
                if (reweighMaxPitch) {
                    LpsSaWeighReqstChannel req;
                    req.command = LpsSaWeighReqstChannel::Command::WRITE_REWEIGH_MAX_PITCH;
                    req.arg.f1 = *reweighMaxPitch;
                    weighAppInf_.sendRequest(req);
                }
            }
            { // reweighMinPitch
                auto reweighMinPitch = pt.get_optional<float>("reweighMinPitch");
                if (reweighMinPitch) {
                    LpsSaWeighReqstChannel req;
                    req.command = LpsSaWeighReqstChannel::Command::WRITE_REWEIGH_MIN_PITCH;
                    req.arg.f1 = *reweighMinPitch;
                    weighAppInf_.sendRequest(req);
                }
            }
            { // reweighMaxAbsRoll
                auto reweighMaxAbsRoll = pt.get_optional<float>("reweighMaxAbsRoll");
                if (reweighMaxAbsRoll) {
                    LpsSaWeighReqstChannel req;
                    req.command = LpsSaWeighReqstChannel::Command::WRITE_REWEIGH_MAX_ABS_ROLL;
                    req.arg.f1 = *reweighMaxAbsRoll;
                    weighAppInf_.sendRequest(req);
                }
            }
            { // reweighMinLiftCylVel
                auto reweighMinLiftCylVel = pt.get_optional<float>("reweighMinLiftCylVel");
                if (reweighMinLiftCylVel) {
                    LpsSaWeighReqstChannel req;
                    req.command = LpsSaWeighReqstChannel::Command::WRITE_REWEIGH_MIN_LIFT_CYL_VEL;
                    req.arg.f1 = *reweighMinLiftCylVel;
                    weighAppInf_.sendRequest(req);
                }
            }
            { // imuCompEnabled
                auto imuCompEnabled = pt.get_optional<bool>("imuCompEnabled");
                if (imuCompEnabled) {
                    LpsSaWeighReqstChannel req;
                    req.command = LpsSaWeighReqstChannel::Command::WRITE_IMU_COMP_ENABLE;
                    req.arg.b = *imuCompEnabled;
                    weighAppInf_.sendRequest(req);
                }
            }
        }

        return true;
    }

    return false;
}

/*
 * Function: processFileRequestPayloadServiceDashboard
 * Purpose: Provide calibration and configuration files to the service dashboard.
 */
bool AisJhm2RequestProcessor::processFileRequestPayloadServiceDashboard(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp)
{
    LpsSaWeighReqstChannel request;
    LpsSaWeighRespChannel response;

    if ("LiftSensorCalibration" == file) {
        request.command = LpsSaWeighReqstChannel::Command::PUBLISH_LIFT_SENSOR_CALIBRATION;
    }
    else if ("TiltSensorCalibration" == file) {
        request.command = LpsSaWeighReqstChannel::Command::PUBLISH_TILT_SENSOR_CALIBRATION;
    }
    else if ("WeighCalibration" == file) {
        request.command = LpsSaWeighReqstChannel::Command::PUBLISH_WEIGH_CALIBRATION;
    }
    else if ("WeighConfiguration" == file) {
        request.command = LpsSaWeighReqstChannel::Command::PUBLISH_WEIGH_CONFIGURATION;
    }
    else if ("RecentWeighResults" == file) {
        request.command = LpsSaWeighReqstChannel::Command::PUBLISH_RECENT_WEIGH_RESULTS;
    }

    if ((LpsSaWeighReqstChannelStorage::Command::NONE != request.command) &&
            weighAppInf_.sendRequestGetResponse(request, response) && response.success) {
        tes_common_ais::IFlocker ifl(response.filePath());
        if (ifl) {
            // Read the file and send it.
            std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
            std::string contents{ begin, end };
            ifl.close();
            resp.contents(contents);
            AIS_LOG_INFO("Responding with %s JSON.", file.c_str());
            return true;
        }
        else {
            resp.contents(R"({"status":"error"})");
            AIS_LOG_ERROR("%s JSON file could not be opened and locked for reading.", file.c_str());
            return true;
        }
    }
    else {
        resp.contents(R"({"status":"error"})");
        AIS_LOG_ERROR("%s could not be published.", file.c_str());
        return true;
    }
}

