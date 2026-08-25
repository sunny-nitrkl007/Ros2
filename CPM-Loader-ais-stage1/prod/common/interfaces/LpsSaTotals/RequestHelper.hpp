#ifndef LPSSATOTALS_REQUESTHELPER_HPP
#define LPSSATOTALS_REQUESTHELPER_HPP

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <ais/task/InterfaceDb.h>
#include <ais/log/Logger.h>

#include <boost/signals2.hpp>

#include <interfaces/LpsSaTotals/RequestInterface.hpp>
#include <interfaces/LpsSaTotals/RequestInterfaceOutputChannel.h>

#include <interfaces/LpsSaTotals/ResponseInterface.hpp>
#include <interfaces/LpsSaTotals/ResponseInterfaceInputChannel.h>

class LpsSaTotalsRequestHelper {
public:
    static constexpr std::chrono::milliseconds timeoutDurationDefault() { return std::chrono::milliseconds(2000); }

    LpsSaTotalsRequestHelper() :
        appName_(),
        mtx_(),
        lastRequestId_(0),
        lastCommand_(LpsSaTotalsRequestInterfaceCommand::NONE),
        responseDataCv_(),
        responseData_(),
        requestOutput_(nullptr),
        responseInput_(nullptr),
        responseInputConnection_() {}

    virtual ~LpsSaTotalsRequestHelper() {
        stop();
    }

    /*
     * Start the interface service.
     */
    bool start(const std::string& appName,
            const std::string& requestOutputName,
            const std::string& responseInputName) {
        bool everythingOk = true;

        LpsSaTotalsRequestInterfaceOutputChannel* requestOutput = nullptr;
        if (!task::InterfaceDb::bind(requestOutputName, requestOutput)) {
            AIS_LOG_ERROR("%s Interface not configured.", requestOutputName.c_str());
            everythingOk = false;
        }

        LpsSaTotalsResponseInterfaceInputChannel* responseInput = nullptr;
        if (!task::InterfaceDb::bind(responseInputName, responseInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", responseInputName.c_str());
            everythingOk = false;
        }

        if (everythingOk) {
            if (!start(appName, requestOutput, responseInput)) {
                AIS_LOG_ERROR("Failed to start list manager request helper.");
                everythingOk = false;
            }
        }

        return everythingOk;
    }


    /*
     * Start the interface service.
     */
    bool start(const std::string& appName,
            LpsSaTotalsRequestInterfaceOutputChannel* requestOutput,
            LpsSaTotalsResponseInterfaceInputChannel* responseInput) {

        stop();

        appName_ = appName;

        // Reinitialize these if this is a "restart"
        lastRequestId_ = 0;
        lastCommand_ = LpsSaTotalsRequestInterfaceCommand::NONE;

        requestOutput_ = requestOutput;
        responseInput_ = responseInput;

        // Attach a listener to the responseInput channel
        if (nullptr != responseInput_) {
            responseInputConnection_ = responseInput_->addNewDataSlot([this]{
                notifyResponseInput();
            });
        }

        // Everything is successful if we connected our listeners and have a requestOutput
        return (nullptr != requestOutput) && (responseInputConnection_.connected());
    }

    /*
     * Stop the interface service.
     */
    bool stop() {
        // Disconnect the responseInput listener if connected.
        if (nullptr != responseInput_) {
            if (responseInputConnection_.connected()) {
                responseInput_->removeNewDataSlot(responseInputConnection_);
            }
        }

        return true;
    }

    /*
     * Send a request and set up to ignore incoming tx data until either
     * the request has been handled by the weigh app, or a timeout has elapsed.
     */
    bool sendRequest(LpsSaTotalsRequestInterface& request) {
        bool success = false;

        if (nullptr != requestOutput_) {
            request.appName = appName_;
            request.appRequestId = LpsSaTotalsRequestInterface::getNextAppRequestId();

            if (requestOutput_->publish(request)) {
                std::lock_guard<std::mutex> lck(mtx_);
                lastRequestId_ = request.appRequestId;
                lastCommand_ = request.command;
                success = true;
            }
        }

        return success;
    }

    /*
     * Wait for a response to the most recent request.
     */
    bool waitForResponse(const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        std::unique_lock<std::mutex> lck(mtx_);
        if (LpsSaTotalsRequestInterfaceCommand::NONE != lastCommand_) {
            // We are awaiting a response, use a condition variable to wait for the new
            // data or a timeout.
            if (std::cv_status::no_timeout == responseDataCv_.wait_for(lck, timeoutDuration)) {
                success = true;
            }
        }
        else {
            success = true;
        }

        return success;
    }

    /*
     * Send a request and wait for the response
     */
    bool sendRequestWaitForResponse(LpsSaTotalsRequestInterface& request,
            const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        if (sendRequest(request)) {
            if (waitForResponse(timeoutDuration)) {
                success = true;
            }
        }
        return success;
    }

    /*
     * Send a request and wait for the response
     */
    bool sendRequestGetResponse(LpsSaTotalsRequestInterface& request,
            LpsSaTotalsResponseInterface& response,
            const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        if (sendRequest(request)) {
            if (waitForResponse(timeoutDuration)) {
                getLastResponse(response);
                success = true;
            }
        }
        return success;
    }


    /*
     * Get the last received response.
     */
    void getLastResponse(LpsSaTotalsResponseInterface& response) {
        std::lock_guard<std::mutex> lck(mtx_);
        response = responseData_;
    }

    /*
     * Get the last received response.
     */
    LpsSaTotalsResponseInterface getLastResponse() {
        std::lock_guard<std::mutex> lck(mtx_);
        return responseData_;
    }


protected:
    void notifyResponseInput() {
        if (nullptr != responseInput_) {
            LpsSaTotalsResponseInterface response;
            while (responseInput_->get(response)) {
                std::unique_lock<std::mutex> lck(mtx_);
                responseData_ = response;
                if ((response.appName == appName_) &&
                        (response.appRequestId == lastRequestId_) &&
                        (response.command == lastCommand_)) {
                    lastCommand_ = LpsSaTotalsRequestInterfaceCommand::NONE;
                    lck.unlock();
                    responseDataCv_.notify_all();
                }
            }
        }
    }

private:

    std::string appName_;

    std::mutex mtx_;
    uint32_t lastRequestId_;
    LpsSaTotalsRequestInterfaceCommand lastCommand_;
    std::condition_variable responseDataCv_;
    LpsSaTotalsResponseInterface responseData_;

    LpsSaTotalsRequestInterfaceOutputChannel* requestOutput_;
    LpsSaTotalsResponseInterfaceInputChannel* responseInput_;

    boost::signals2::connection responseInputConnection_;

};

#endif
