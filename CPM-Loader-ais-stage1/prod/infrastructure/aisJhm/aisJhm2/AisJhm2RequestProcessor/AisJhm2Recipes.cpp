/*
 * AisJhm2Recipes.cpp
 */
#include <chrono/print.hpp>
#include <chrono/tz.hpp>

#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"
#include "AisJhm2UintParam.h"
#include "AisJhm2BoolParam.h"

#define PAYLOAD_DETAILS_FILEPATH_JSON \
    (R"(/tmp/appdata/CPM/LpsSaJobMgrApp/PayloadDetails.json)")

void AisJhm2RequestProcessor::installRecipeHandlers() {
    fileMsgRequestHandlerMap_["PayloadDetails"] = &AisJhm2RequestProcessor::processFileRequestPayloadDetails;

    cmdMsgRequestHandlerMap_["TotalTargetWeight_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadDetailsWr;
    cmdMsgRequestHandlerMap_["TargetType_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadDetailsWr;
    cmdMsgRequestHandlerMap_["SubtotalInfo_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadDetailsWr;
    cmdMsgRequestHandlerMap_["SelectSubtotalByIndex_Wr"] = &AisJhm2RequestProcessor::processCmdRequestPayloadDetailsWr;
    cmdMsgRequestHandlerMap_["DeleteRecipe_Wr"] = &AisJhm2RequestProcessor::processCmdRequestDeleteRecipeWr;
    cmdMsgRequestHandlerMap_["SaveRecipe_Wr"] = &AisJhm2RequestProcessor::processCmdRequestSaveRecipeWr;
    cmdMsgRequestHandlerMap_["SelectRecipe_Wr"] = &AisJhm2RequestProcessor::processCmdRequestSelectRecipeWr;
}

std::string recipeName = "my_test_recipe";

bool AisJhm2RequestProcessor::processCmdRequestPayloadDetailsWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    if ("TotalTargetWeight_Wr" == cmd) {
        auto totaTargetWeight = pt.get_value_optional<float>();
        LpsSaJobMgrReqstChannel req;
        if (totaTargetWeight) {
            req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TOTAL_TARGET_WEIGHT;
            req.data.totalTargetWeight = *totaTargetWeight;
            jobMgrHelper_.sendRequest(req);
            AIS_LOG_INFO("TotalTargetWeight_Wr received");
            return true;
        }
    }

    if ("TargetType_Wr" == cmd) {
        auto targetType = pt.get_value_optional<uint>();
        LpsSaJobMgrReqstChannel req;
        if (targetType) {
            req.command = LpsSaJobMgrReqstChannel::Command::WRITE_TARGET_TYPE;
            req.data.targetType = *targetType;
            jobMgrHelper_.sendRequest(req);
            AIS_LOG_INFO("TargetType_Wr received: %d", *targetType);
            return true;
        }
    }

    if ("SelectSubtotalByIndex_Wr" == cmd) {
        auto index = pt.get_value_optional<uint>();
        LpsSaJobMgrReqstChannel req;
        if (index) {
            req.command = LpsSaJobMgrReqstChannel::Command::SELECT_SUBTOTAL;
            req.data.subtotalIndex = *index;
            jobMgrHelper_.sendRequest(req);
            AIS_LOG_INFO("SelectSubtotalByIndex_Wr received: %d", *index);
            return true;
        }
    }

    if ("SubtotalInfo_Wr" == cmd) {
        LpsSaJobMgrReqstChannel req;

        auto command = pt.get_optional<std::string>("command");
        auto currentStepNumber = pt.get_optional<uint16_t>("currentStepNumber");
        auto newStepNumber = pt.get_optional<uint16_t>("newStepNumber");
        auto targetWeight = pt.get_optional<float>("targetWeight");
        auto targetProportion = pt.get_optional<uint16_t>("targetProportion");
        auto targetPasses = pt.get_optional<uint16_t>("targetPasses");
        auto materialId = pt.get_optional<uint32_t>("materialId");
        auto materialName = pt.get_optional<std::string>("materialName");
        auto materialDensity = pt.get_optional<float>("materialDensity");
        auto icontype = pt.get_optional<uint8_t>("iconType");

        req.command = LpsSaJobMgrReqstChannel::Command::WRITE_SUBTOTAL_INFO;
        if (command) {
            req.data.subtotalInfo.command = *command;
        }
        else {
            AIS_LOG_ERROR("SubtotalInfo_Wr received without a command(INSERT, EDIT, DELETE.");
            return false;
        }

        // send the data we received to jobmgr
        if (currentStepNumber) {
            req.data.subtotalInfo.currentStepNumber = *currentStepNumber;
        }
        if (newStepNumber) {
            req.data.subtotalInfo.newStepNumber = *newStepNumber;
        }
        if (targetWeight) {
            req.data.subtotalInfo.targetWeight = *targetWeight;
        }
        if (targetProportion) {
            req.data.subtotalInfo.targetProportion = *targetProportion;
        }
        if (targetPasses) {
            req.data.subtotalInfo.targetPasses = *targetPasses;
        }
        if (materialId) {
            req.data.subtotalInfo.materialId = *materialId;
        }
        if (materialName) {
            req.data.subtotalInfo.materialName = *materialName;
        }
        if (materialDensity) {
            req.data.subtotalInfo.materialDensity = *materialDensity;
        }
        if (icontype) {
            req.data.subtotalInfo.iconType = *icontype;
        }

        jobMgrHelper_.sendRequest(req);
        AIS_LOG_INFO("SubtotalInfo_Wr received and sent to jobmgr");
        return true;
    }

    return false;
}

bool AisJhm2RequestProcessor::processFileRequestPayloadDetails(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {

    std::string payloadDetailsFileName(PAYLOAD_DETAILS_FILEPATH_JSON);

    AIS_LOG_INFO("Payload Details JSON file requested");

    { // Send request to create file
        LpsSaJobMgrReqstChannel req;
        req.command = LpsSaJobMgrReqstChannel::Command::PAYLOAD_DETAILS_FILE_REQUEST;
        jobMgrHelper_.sendRequestWaitForResponse(req);
    }

    // Send file
    tes_common_ais::IFlocker ifl(payloadDetailsFileName);
    if (ifl) {
        // Read the file and send it.
        std::istreambuf_iterator<char> begin(ifl.ifstream()), end;
        std::string contents{ begin, end };
        ifl.close();
        resp.contents(contents);
        AIS_LOG_INFO("Responding with Payload Details JSON.");

        // Delete the file since we are done with it


        return true;
    }
    else {
        resp.contents(R"({"status":"error"})");
        AIS_LOG_ERROR("Payload Details JSON file could not be opened or is locked for reading.");
        return true;
    }
    return false;
}

bool AisJhm2RequestProcessor::processCmdRequestDeleteRecipeWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "DeleteRecipe_Wr") {
        return false;
    }

    auto recipeName = pt.get_optional<std::string>("recipeName");
    if (recipeName) {
        AIS_LOG_INFO("Delete Recipe Command Received for: %s", (*recipeName).c_str());
    }

    return true;
}

bool AisJhm2RequestProcessor::processCmdRequestSaveRecipeWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "SaveRecipe_Wr") {
        return false;
    }

    auto recipeName = pt.get_optional<std::string>("recipeName");
    if (recipeName) {
        AIS_LOG_INFO("Save Recipe Command Received for: %s", (*recipeName).c_str());
    }

    return true;
}

bool AisJhm2RequestProcessor::processCmdRequestSelectRecipeWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    if (cmd != "SelectRecipe_Wr") {
        return false;
    }

    auto recipeName = pt.get_optional<std::string>("recipeName");
    if (recipeName) {
        AIS_LOG_INFO("Select Recipe Command Received for: %s", (*recipeName).c_str());
    }

    return true;
}
