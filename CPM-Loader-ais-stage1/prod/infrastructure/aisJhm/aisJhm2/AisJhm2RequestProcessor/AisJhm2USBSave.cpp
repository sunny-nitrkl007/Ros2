/*
 * AisJhm2USB.cpp
 *
 *  Created on: March 04, 2022
 *      Author: pf
 */

#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"


// USB support
/******************************************************************************
FUNCTION: processParamRequestUSBSaveProgress
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestUSBSaveProgress(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    
    AisJhm2MultiParam* m = new AisJhm2MultiParam("USBSaveProgress");
 
    /* USBSaveProgress Request
     * state -
     *     0 - Save in Process
     *     1 - Save Complete
     *     2 - Error: Generic
     *     3 - Error: Insufficient Space
     *     4 - Error: No USB Device Detected
     *     5 - Save Cancelled
     * totalFiles -
     * filesTransfered -
     * */

    uint8_t state_ = 2;
    int totalFiles_ = 0;
    int filesTransferred_ = 0;

    // send request and wait for response. Then receive the USBSaveProgress
    LpsSaTotalsRequestInterface request;
    request.command = LpsSaTotalsRequestInterfaceCommand::USB_SAVE_PROGRESS;
    LpsSaTotalsResponseInterface response;
    if (totalsRequestHelper_.sendRequestGetResponse(request, response) && response.success) {
        // receive USB Save Progress.
        if (nullptr != USBSaveProgressInput_) {
            LpsSaTotalsUSBSaveProgressInterface saveProgress;
            while (USBSaveProgressInput_->get(saveProgress)) {
                state_ = saveProgress.state;
                totalFiles_ = saveProgress.totalFiles;
                filesTransferred_ = saveProgress.filesTransfered;
            }
        }
    }
    else {
        AIS_LOG_ERROR("USBSaveProgress not received from TotalsApp");
        return false;
    }

    if (nullptr != m) {

        { // state (uint8_t)
            m->addParameter(Jhm2SimpleParam("state", state_));
        }

        { // totalFiles (uint32_t)
            m->addParameter(Jhm2SimpleParam("totalFiles", /*DSI*/ 0, totalFiles_));
        }

        { // filesTransfered (uint32_t)
            m->addParameter(Jhm2SimpleParam("filesTransfered", /*DSI*/ 0, filesTransferred_));
        }
       
        resp.insertParam(m);

        return true;
    }
    else {
        AIS_LOG_ERROR("USBSaveProgress Param Request unsuccessful");
        return false;
    }
}

/******************************************************************************
FUNCTION: processCmdRequestUSBSaveWr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestUSBSaveWr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    auto USBSave = pt.get_value_optional<uint8_t>();

    /* USBSave Command Request
     * 0 - Cancel any save in progress
     * 1 - Save New Data Files
     * 2 - Save All Data Files
     * */

    /* USBSave pointer is not null, update the data */ 
    if (USBSave) {
        uint8_t usbSave_ = *USBSave;

        LpsSaTotalsRequestInterface request;
        if (usbSave_ == 0) {
            request.command = LpsSaTotalsRequestInterfaceCommand::USB_CMD_SAVE_CANCEL;
        }
        else if (usbSave_ == 1) {
            request.command = LpsSaTotalsRequestInterfaceCommand::USB_CMD_SAVE_NEW;
        }
        else if (usbSave_ == 2) {
            request.command = LpsSaTotalsRequestInterfaceCommand::USB_CMD_SAVE_ALL;
        }
        else {
            AIS_LOG_ERROR("Invalid Command Received from UI for USBSaveWr: %d", usbSave_);
            return false;
        }

        LpsSaTotalsResponseInterface response;
        if (totalsRequestHelper_.sendRequestGetResponse(request, response) && response.success) {
            AIS_LOG_ERROR("USBSaveWr command sent and success");
            return true;
        }
    }

    // Something went wrong
    AIS_LOG_ERROR("USBSaveWr failed");

    return false;
}

