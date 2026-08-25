/*
 * AisJhm2Totals.cpp
 *
 *  Created on: May 2, 2022
 *      Author: pf
 */
#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"

void AisJhm2RequestProcessor::installTotalsHandlers() {
    paramMsgRequestHandlerMap_["LifetimeTotals"] = &AisJhm2RequestProcessor::processParamRequestLifetimeTotals;
    paramMsgRequestHandlerMap_["TripATotals"] = &AisJhm2RequestProcessor::processParamRequestTripATotals;
    paramMsgRequestHandlerMap_["TripBTotals"] = &AisJhm2RequestProcessor::processParamRequestTripBTotals;
    paramMsgRequestHandlerMap_["AllTotals"] = &AisJhm2RequestProcessor::processParamRequestAllTotals;

    cmdMsgRequestHandlerMap_["PrintedReports_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestPrintedReportsClr;
    cmdMsgRequestHandlerMap_["TruckReport_Clr"] = &AisJhm2RequestProcessor::processCmdRequestTruckReportClr;
    cmdMsgRequestHandlerMap_["TruckReport_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestTruckReportClr;
    cmdMsgRequestHandlerMap_["MaterialTotals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["MaterialTotals_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["MaterialTotals_Req"] = &AisJhm2RequestProcessor::processCmdRequestMaterialTotalsReq; // deprecated
    cmdMsgRequestHandlerMap_["TruckTotals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["TruckTotals_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag1Totals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag1Totals_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag2Totals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag2Totals_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag3Totals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag3Totals_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag4Totals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["Tag4Totals_ClrAll"] = &AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr;
    cmdMsgRequestHandlerMap_["TripATotals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestTripATotalsClr;
    cmdMsgRequestHandlerMap_["TripBTotals_Clr"] = &AisJhm2RequestProcessor::processCmdRequestTripBTotalsClr;

    cmdMsgRequestHandlerMap_["TruckReport_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["MaterialTotals_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["TruckTotals_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["Tag1Totals_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["Tag2Totals_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["Tag3Totals_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["Tag4Totals_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;
    cmdMsgRequestHandlerMap_["Test_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTotals;

    fileMsgRequestHandlerMap_["MaterialTotals"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["TruckTotals"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag1Totals"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag2Totals"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag3Totals"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag4Totals"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;

    fileMsgRequestHandlerMap_["MaterialTotalsToday"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["TruckTotalsToday"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag1TotalsToday"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag2TotalsToday"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag3TotalsToday"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag4TotalsToday"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;

    fileMsgRequestHandlerMap_["MaterialTotalsPreviousDay"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["TruckTotalsPreviousDay"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag1TotalsPreviousDay"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag2TotalsPreviousDay"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag3TotalsPreviousDay"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
    fileMsgRequestHandlerMap_["Tag4TotalsPreviousDay"] = &AisJhm2RequestProcessor::processFileRequestMappedTotals;
}

/******************************************************************************
FUNCTION: processParamRequestLifetimeTotals
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestLifetimeTotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        /*
         * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
         * to strings.  I would rather have control over precision than let Jhm2 pick a fixed implementation
         * for every number, so I am converting things to strings here.
         */

        { // Weight
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << totals_.weightTonnes;
            const std::string& str = ss.str();
            m->addParameter(Jhm2SimpleParam("weight", str));
            AIS_LOG_DEBUG("Lifetime Total Weight: '%s'", str.c_str());
        }

        { // Load Count
            const std::string& str = std::to_string(totals_.loadCount);
            m->addParameter(Jhm2SimpleParam("loadCount", str));
            AIS_LOG_DEBUG("Lifetime Total Load Count: '%s'", str.c_str());
        }

        { // Pass Count
            const std::string& str = std::to_string(totals_.passCount);
            m->addParameter(Jhm2SimpleParam("passCount", str));
            AIS_LOG_DEBUG("Lifetime Total Pass Count: '%s'", str.c_str());
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestTripATotals
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestTripATotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        /*
         * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
         * to strings.  I would rather have control over precision than let Jhm2 pick a fixed implementation
         * for every number, so I am converting things to strings here.
         */

        { // Weight
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << totals_.trips[0].weightTonnes;
            const std::string& str = ss.str();
            m->addParameter(Jhm2SimpleParam("weight", str));
            AIS_LOG_DEBUG("Trip A Weight: '%s'", str.c_str());
        }

        { // Load Count
            const std::string& str = std::to_string(totals_.trips[0].loadCount);
            m->addParameter(Jhm2SimpleParam("loadCount", str));
            AIS_LOG_DEBUG("Trip A Load Count: '%s'", str.c_str());
        }

        { // Pass Count
            const std::string& str = std::to_string(totals_.trips[0].passCount);
            m->addParameter(Jhm2SimpleParam("passCount", str));
            AIS_LOG_DEBUG("Trip A Pass Count: '%s'", str.c_str());
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestTripBTotals
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestTripBTotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    AisJhm2MultiParam* m = new AisJhm2MultiParam(paramName);
    if (nullptr != m) {
        /*
         * Ideally, we would serialize numbers as numbers, but the underlying Jhm2 crap converts everything
         * to strings.  I would rather have control over precision than let Jhm2 pick a fixed implementation
         * for every number, so I am converting things to strings here.
         */

        { // Weight
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << totals_.trips[1].weightTonnes;
            const std::string& str = ss.str();
            m->addParameter(Jhm2SimpleParam("weight", str));
            AIS_LOG_DEBUG("Trip B Weight: '%s'", str.c_str());
        }

        { // Load Count
            const std::string& str = std::to_string(totals_.trips[1].loadCount);
            m->addParameter(Jhm2SimpleParam("loadCount", str));
            AIS_LOG_DEBUG("Trip B Load Count: '%s'", str.c_str());
        }

        { // Pass Count
            const std::string& str = std::to_string(totals_.trips[1].passCount);
            m->addParameter(Jhm2SimpleParam("passCount", str));
            AIS_LOG_DEBUG("Trip B Pass Count: '%s'", str.c_str());
        }

        resp.insertParam(m);

        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processParamRequestAllTotals
DESCRIPTION:
    Populates the parameter request response message with the response.
PARAMETER DESCRIPTION:
    paramName - Requested Parameter Name
    resp - Parameter Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processParamRequestAllTotals(const std::string& paramName, const Jhm2ParamMsgRequest& req, Jhm2ParamMsg& resp) {
    processParamRequestLifetimeTotals("LifetimeTotals", req, resp);
    processParamRequestTripATotals("TripATotals", req, resp);
    processParamRequestTripBTotals("TripBTotals", req, resp);
    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestPrintedReportsClr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestPrintedReportsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaTotalsRequestInterface req;
    auto oArg = pt.get_value_optional<std::string>();

    if ("PrintedReports_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_ALL_PRINTED_REPORTS;
    }

    // Send the request if there is one
    if (LpsSaTotalsRequestInterfaceCommand::NONE != req.command) {
        totalsRequestHelper_.sendRequestWaitForResponse(req);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestTruckReportClr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTruckReportClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaTotalsRequestInterface req;
    auto oArg = pt.get_value_optional<std::string>();

    if ("TruckReport_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TRUCK_REPORT_ALL;
    }
    else if ("TruckReport_Clr" == cmd) {
        if (oArg) {
            const std::string& arg = *oArg;
            if (arg.empty()) {
                // This is just for backwards compatibility
                req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TRUCK_REPORT_ALL;
            }
            else {
                /* Not Implemented */
                AIS_LOG_WARN("Clearing an individual truck from a truck report is not supported.");
            }
        }
    }

    // Send the request if there is one
    if (LpsSaTotalsRequestInterfaceCommand::NONE != req.command) {
        totalsRequestHelper_.sendRequestWaitForResponse(req);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestMaterialTotalsReq
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestMaterialTotalsReq(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    // Do nothing.  The file request handler now requests the file to be published.
    return true;
}


/******************************************************************************
FUNCTION: processCmdRequestTripATotalsClr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTripATotalsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    auto oArg = pt.get_value_optional<bool>();
    if (oArg && *oArg) {
        LpsSaTotalsRequestInterface req;
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TRIP_TOTAL_A;
        totalsRequestHelper_.sendRequestWaitForResponse(req);
    }
    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestTripBTotalsClr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestTripBTotalsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    auto oArg = pt.get_value_optional<bool>();
    if (oArg && *oArg) {
        LpsSaTotalsRequestInterface req;
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TRIP_TOTAL_B;
        totalsRequestHelper_.sendRequestWaitForResponse(req);
    }
    return true;
}

/******************************************************************************
FUNCTION: processCmdRequestMaterialTotalsClr
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestMappedTotalsClr(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {

    LpsSaTotalsRequestInterface req;
    auto oArg = pt.get_value_optional<std::string>();

    if ("MaterialTotals_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_MATERIAL_TOTALS_ALL;
    }
    else if ("MaterialTotals_Clr" == cmd) {
        if (oArg) {
            req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_MATERIAL_TOTAL;
            req.materialName(*oArg);
        }
    }
    else if ("TruckTotals_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TRUCK_TOTALS_ALL;
    }
    else if ("TruckTotals_Clr" == cmd) {
        if (oArg) {
            req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TRUCK_TOTAL;
            req.truckName(*oArg);
        }
    }
    else if ("Tag1Totals_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTALS_ALL;
        req.tagTotalsIndex(1);
    }
    else if ("Tag1Totals_Clr" == cmd) {
        if (oArg) {
            req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTAL;
            req.tagTotalsIndex(1);
            req.tagItemName(*oArg);
        }
    }
    else if ("Tag2Totals_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTALS_ALL;
        req.tagTotalsIndex(2);
    }
    else if ("Tag2Totals_Clr" == cmd) {
        if (oArg) {
            req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTAL;
            req.tagTotalsIndex(2);
            req.tagItemName(*oArg);
        }
    }
    else if ("Tag3Totals_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTALS_ALL;
        req.tagTotalsIndex(3);
    }
    else if ("Tag3Totals_Clr" == cmd) {
        if (oArg) {
            req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTAL;
            req.tagTotalsIndex(3);
            req.tagItemName(*oArg);
        }
    }
    else if ("Tag4Totals_ClrAll" == cmd) {
        req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTALS_ALL;
        req.tagTotalsIndex(4);
    }
    else if ("Tag4Totals_Clr" == cmd) {
        if (oArg) {
            req.command = LpsSaTotalsRequestInterfaceCommand::CLEAR_TAG_TOTAL;
            req.tagTotalsIndex(4);
            req.tagItemName(*oArg);
        }
    }

    // Send the request if there is one
    if (LpsSaTotalsRequestInterfaceCommand::NONE != req.command) {
        totalsRequestHelper_.sendRequestWaitForResponse(req);
        return true;
    }

    return false;
}

/******************************************************************************
FUNCTION: processCmdRequestPrint
DESCRIPTION:
    Executes the command request.
PARAMETER DESCRIPTION:
    cmd - Command Name
    pt - Property tree containing data
    resp - Command Request Response Message
RETURN VALUE:
    true if successful, else false
*******************************************************************************/
bool AisJhm2RequestProcessor::processCmdRequestPrintTotals(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    auto oArg = pt.get_value_optional<std::string>();
    if (oArg) {
        bool sendRequest = true;
        LpsSaTotalsRequestInterface req;

        if ("MaterialTotals_Prt" == cmd) {
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_MATERIAL_TOTALS_ALL;
            }
            else if (1 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_MATERIAL_TOTALS_TODAY;
            }
            else if (2 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_MATERIAL_TOTALS_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                /* Not Implemented */
                AIS_LOG_WARN("Printing material total of this type is not supported.");
            }
        }
        else if ("TruckReport_Prt" == cmd) {
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_REPORT_ALL;
            }
            else if (1 == cmdType) {
                // The implementation for this was missed!
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_REPORT_TODAY ;
            }
            else if (2 == cmdType) {
                // The implementation for this was missed!
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_REPORT_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                AIS_LOG_WARN("Printing a truck report of this type is not supported.");
            }
        }
        else if ("TruckTotals_Prt" == cmd) {
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_TOTALS_ALL;
            }
            else if (1 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_TOTALS_TODAY;
            }
            else if (2 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_TOTALS_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                /* Not Implemented */
                AIS_LOG_WARN("Printing truck total of this type is not supported.");
            }
        }
        else if ("Tag1Totals_Prt" == cmd) {
            req.tagTotalsIndex(1);
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_ALL;
            }
            else if (1 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_TODAY;
            }
            else if (2 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                AIS_LOG_WARN("Printing tag1 totals of this type not supported.");
            }
        }
        else if ("Tag2Totals_Prt" == cmd) {
            req.tagTotalsIndex(2);
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_ALL;
            }
            else if (1 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_TODAY;
            }
            else if (2 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                AIS_LOG_WARN("Printing tag2 totals of this type not supported.");
            }
        }
        else if ("Tag3Totals_Prt" == cmd) {
            req.tagTotalsIndex(3);
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_ALL;
            }
            else if (1 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_TODAY;
            }
            else if (2 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                AIS_LOG_WARN("Printing tag3 totals of this type not supported.");
            }
        }
        else if ("Tag4Totals_Prt" == cmd) {
            req.tagTotalsIndex(4);
            auto cmdType = pt.get_value<uint8_t>(0);
            if (0 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_ALL;
            }
            else if (1 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_TODAY;
            }
            else if (2 == cmdType) {
                req.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TAG_TOTALS_PREVIOUS_DAY;
            }
            else {
                sendRequest = false;
                AIS_LOG_WARN("Printing tag4 totals of this type not supported.");
            }
        }
        else if ("Test_Prt" == cmd) {
            /*
             * This command is sent directly to the printer to print the test page.
             */

            // Default the number of copies to 1
            uint8_t numCopies = pt.get_value<uint8_t>(1);

            // Limit the number of copies to 3
            if (numCopies > 3) {
                numCopies = 3;
            }

            // Build a request to the printer app to do a test print.
            if (nullptr != printerRequestChannelOutput_) {
                SerialPrinterRequestInterface prtReq;
                prtReq.command = SerialPrinterRequestInterfaceCommand::TEST;
                prtReq.numberOfCopies(numCopies);
                printerRequestChannelOutput_->publish(prtReq);
            }

            // Don't send a request to the totals app.
            sendRequest = false;
        }
        else {
            sendRequest = false;
        }

        if (sendRequest) {
            totalsRequestHelper_.sendRequestWaitForResponse(req);
        }
    }
    return true;
}

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
bool AisJhm2RequestProcessor::processFileRequestMappedTotals(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    LpsSaTotalsRequestInterface request;

    if (file.rfind("Material", 0) == 0) {
        if ("MaterialTotals" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_MATERIAL_TOTALS;
        }
        else if ("MaterialTotalsToday" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_MATERIAL_TOTALS_TODAY;
        }
        else if ("MaterialTotalsPreviousDay" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_MATERIAL_TOTALS_PREVIOUS_DAY;
        }
    }
    else if (file.rfind("Truck", 0) == 0) {
        if ("TruckTotals" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TRUCK_TOTALS;
        }
        else if ("TruckTotalsToday" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TRUCK_TOTALS_TODAY;
        }
        else if ("TruckTotalsPreviousDay" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TRUCK_TOTALS_PREVIOUS_DAY;
        }
    }
    else if (file.rfind("Tag1", 0) == 0) {
        request.tagTotalsIndex(1);
        if ("Tag1Totals" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS;
        }
        else if ("Tag1TotalsToday" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_TODAY;
        }
        else if ("Tag1TotalsPreviousDay" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_PREVIOUS_DAY;
        }
    }
    else if (file.rfind("Tag2", 0) == 0) {
        request.tagTotalsIndex(2);
        if ("Tag2Totals" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS;
        }
        else if ("Tag2TotalsToday" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_TODAY;
        }
        else if ("Tag2TotalsPreviousDay" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_PREVIOUS_DAY;
        }
    }
    else if (file.rfind("Tag3", 0) == 0) {
        request.tagTotalsIndex(3);
        if ("Tag3Totals" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS;
        }
        else if ("Tag3TotalsToday" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_TODAY;
        }
        else if ("Tag3TotalsPreviousDay" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_PREVIOUS_DAY;
        }
    }
    else if (file.rfind("Tag4", 0) == 0) {
        request.tagTotalsIndex(4);
        if ("Tag4Totals" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS;
        }
        else if ("Tag4TotalsToday" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_TODAY;
        }
        else if ("Tag4TotalsPreviousDay" == file) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TAG_TOTALS_PREVIOUS_DAY;
        }
    }

    if (LpsSaTotalsRequestInterfaceCommand::NONE != request.command) {
        LpsSaTotalsResponseInterface response;
        if (totalsRequestHelper_.sendRequestGetResponse(request, response) && response.success) {
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

    AIS_LOG_ERROR("%s not supported.", file.c_str());
    return false;
}



