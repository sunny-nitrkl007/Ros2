/*
 * AisJhm2Ticket.cpp
 *
 *  Created on: Jan 4, 2023
 *      Author: pf
 */
#include "AisJhm2RequestProcessor.h"
#include "AisJhm2MultiParam.h"

void AisJhm2RequestProcessor::installTicketHandlers() {
    cmdMsgRequestHandlerMap_["StoredTicket_Prt"] = &AisJhm2RequestProcessor::processCmdRequestPrintTicket;

    fileMsgRequestHandlerMap_["TicketHistory"] = &AisJhm2RequestProcessor::processFileRequestTicketHistory;
    fileMsgRequestHandlerMap_["TicketDetails"] = &AisJhm2RequestProcessor::processFileRequestTicketDetails;
}

bool AisJhm2RequestProcessor::processCmdRequestPrintTicket(const std::string& cmd, const boost::property_tree::ptree& pt, Jhm2CmdResponseMsg& resp) {
    LpsSaTotalsRequestInterface request;
    auto oArg = pt.get_value_optional<std::string>();

    if ("StoredTicket_Prt" == cmd) {
        if (oArg) {
            request.command = LpsSaTotalsRequestInterfaceCommand::PRINT_TRUCK_TICKET;
            request.ticketId(*oArg);
        }
    }

    if (LpsSaTotalsRequestInterfaceCommand::NONE != request.command) {
        totalsRequestHelper_.sendRequestWaitForResponse(request);
        return true;
    }

    return false;
}

bool AisJhm2RequestProcessor::processFileRequestTicketHistory(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    LpsSaTotalsRequestInterface request;

    if ("TicketHistory" == file) {
        { // Get "page" query parameter
            const std::set<std::string>& qp = req.getReqData("page");
            if (qp.empty()) {
                request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TICKET_HISTORY;
            }
            else {
                try {
                    std::string page = *qp.cbegin();
                    if (page.empty() || ("this" == page)) {
                        request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TICKET_HISTORY;
                    }
                    else if ("next" == page) {
                        request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TICKET_HISTORY_NEXT;
                    }
                    else if (page.rfind("prev", 0) == 0) {
                        request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TICKET_HISTORY_PREVIOUS;
                    }
                    else {
                        request.command = LpsSaTotalsRequestInterfaceCommand::NONE;
                    }
                }
                catch (...) {
                    request.command = LpsSaTotalsRequestInterfaceCommand::NONE;
                }
            }
        }

        { // Get "localDate" query parameter
            const std::set<std::string>& qp = req.getReqData("localDate");
            if (!qp.empty()) {
                try {
                    request.localDateStr(*qp.cbegin());
                }
                catch (...) {
                    request.command = LpsSaTotalsRequestInterfaceCommand::NONE;
                }
            }
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

bool AisJhm2RequestProcessor::processFileRequestTicketDetails(const std::string& file, const Jhm2ParamMsgRequest& req, AisJhm2FileMsg& resp) {
    LpsSaTotalsRequestInterface request;

    if ("TicketDetails" == file) {
        request.command = LpsSaTotalsRequestInterfaceCommand::PUBLISH_TICKET_DETAILS;

        { // Get "ticketId" query parameter
            const std::set<std::string>& qp = req.getReqData("ticketId");
            if (!qp.empty()) {
                try {
                    request.ticketId(*qp.cbegin());
                }
                catch (...) {
                    request.command = LpsSaTotalsRequestInterfaceCommand::NONE;
                }
            }
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


