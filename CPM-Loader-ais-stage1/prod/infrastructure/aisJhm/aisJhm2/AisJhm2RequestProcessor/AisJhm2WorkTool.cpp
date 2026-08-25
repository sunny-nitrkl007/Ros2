/*
 * AisJhm2WorkTool.cpp
 *
 *  Created on: Jul 21, 2023
 *      Author: pf
 */
#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"
#include "AisJhm2UintParam.h"
#include "AisJhm2BoolParam.h"

void AisJhm2RequestProcessor::installWorkToolHandlers() {
    paramMsgRequestHandlerMap_["WorkToolInfo"] = &AisJhm2RequestProcessor::processParamRequestWorkToolInfo;
    cmdMsgRequestHandlerMap_["WorkToolInfo_Wr"] = &AisJhm2RequestProcessor::processCmdRequestWorkToolInfoWr;
}

bool AisJhm2RequestProcessor::processParamRequestWorkToolInfo(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam("WorkToolInfo");
    if (nullptr != m) {
        m->addParameter(Jhm2SimpleParam("id", weighAppTxData_.WorkToolId));

        resp.insertParam(m);

        return true;
    }
    else {
        AIS_LOG_ERROR("WorkToolInfo Parameter Request unsuccessful");
        return false;
    }
}

bool AisJhm2RequestProcessor::processCmdRequestWorkToolInfoWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "WorkToolInfo_Wr") {
        return false;
    }

    // Must be in service mode
    if (!displayState_.isInServiceMode()) {
        return false;
    }

    bool weighAppReqMade = false;

    if (!legalForTradeInstalled_ || !weighAppTxData_.LftSealStatus.sealed) {
        { // id
            auto id = pt.get_optional<std::string>("id");
            if (id) {
                LpsSaWeighReqstChannel req;
                req.command = LpsSaWeighReqstChannel::Command::WRITE_WORK_TOOL_ID;
                req.arg.s = (*id).substr(0, 20);
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
