/*
 * AisJhm2ServiceHistory.cpp
 *
 *  Created on: May 19, 2023
 *      Author: pf
 */
#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighReqstChannel.h>

#include "AisJhm2RequestProcessor.h"


/******************************************************************************
FUNCTION: processFileRequestMaterialTotals
DESCRIPTION:
    Populates the file request response message with the response.
PARAMETER DESCRIPTION:
    req - Requested File Name
    resp - File Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processFileRequestServiceHistory(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    LpsSaWeighReqstChannel request;
    LpsSaWeighRespChannel response;

    request.command = LpsSaWeighReqstChannel::Command::PUBLISH_SERVICE_HISTORY;

    if (weighAppInf_.sendRequestGetResponse(request, response) && response.success) {
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

