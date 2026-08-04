#ifndef LPSSAWEIGHAPPINF_HPP
#define LPSSAWEIGHAPPINF_HPP

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <boost/signals2.hpp>

#include <interfaces/LpsSaWeighReqstChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighRespChannel/InterfaceTypes.h>
#include <interfaces/LpsSaWeighTxChannel/InterfaceTypes.h>

class LpsSaWeighAppInf {
public:
    static constexpr std::chrono::milliseconds timeoutDurationDefault() { return std::chrono::milliseconds(250); }

    LpsSaWeighAppInf() :
        appName_(),
        mtx_(),
        nextTxTimePoint_(timePointMin_),
        lastRequestId_(0),
        txDataCv_(),
        txData_(),
        lastCommand_(LpsSaWeighReqstChannel::Command::NONE),
        responseDataCv_(),
        responseData_(),
        requestOutput_(nullptr),
        responseInput_(nullptr),
        txInput_(nullptr),
        responseInputConnection_(),
        txInputConnection_() {

        // We have not received any data yet, so initialize this to be as stale as possible.
        txData_.timePoint = timePointMin_;
    }

    virtual ~LpsSaWeighAppInf() {
        stop();
    }

    /* Start the interface service */
    bool start(const std::string& appName,
            const std::string& requestOutputName,
            const std::string& responseInputName,
            const std::string& txInputName) {
        bool everythingOk = true;

        LpsSaWeighReqstChannelOutput* requestOutput = nullptr;
        if (!task::InterfaceDb::bind(requestOutputName, requestOutput)) {
            AIS_LOG_ERROR("%s Interface not configured.", requestOutputName.c_str());
            everythingOk = false;
        }

        LpsSaWeighRespChannelInput* responseInput = nullptr;
        if (!task::InterfaceDb::bind(responseInputName, responseInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", responseInputName.c_str());
            everythingOk = false;
        }

        LpsSaWeighTxChannelInput* txInput;
        if (!task::InterfaceDb::bind(txInputName, txInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", txInputName.c_str());
            everythingOk = false;
        }

        if (everythingOk) {
            if (!start(appName, requestOutput, responseInput, txInput)) {
                AIS_LOG_ERROR("Failed to start request helper.");
                everythingOk = false;
            }
        }

        return everythingOk;
    }

    /* Start the interface service. */
    bool start(const std::string& appName,
            LpsSaWeighReqstChannelOutput* requestOutput,
            LpsSaWeighRespChannelInput* responseInput,
            LpsSaWeighTxChannelInput* txInput) {

        stop();

        // Reinitialize these if this is a "restart"
        nextTxTimePoint_ = timePointMin_;
        lastRequestId_ = 0;
        lastCommand_ = LpsSaWeighReqstChannel::Command::NONE;

        appName_ = appName;
        requestOutput_ = requestOutput;
        responseInput_ = responseInput;
        txInput_ = txInput;

        // Attach a listener to the responseInput channel
        if (nullptr != responseInput_) {
            responseInputConnection_ = responseInput_->addNewDataSlot([this]{
                notifyResponseInput();
            });
        }

        // Attach a listener to the txInput channel
        if (nullptr != txInput_) {
            txInputConnection_ = txInput_->addNewDataSlot([this]{
                notifyTxInput();
            });
        }

        // Everything is successful if we connected our listeners and have a requestOutput
        return (nullptr != requestOutput) &&
                (responseInputConnection_.connected()) &&
                (txInputConnection_.connected());
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

        // Disconnect the txInput listener if connected.
        if (nullptr != txInput_) {
            if (txInputConnection_.connected()) {
                txInput_->removeNewDataSlot(txInputConnection_);
            }
        }

        return true;
    }

    /*
     * Send a request and set up to ignore incoming tx data until either
     * the request has been handled by the weigh app, or a timeout has elapsed.
     */
    bool sendRequest(LpsSaWeighReqstChannel& request) {
        bool success = false;

        if (nullptr != requestOutput_) {
            request.appName = appName_;
            request.appRequestId = LpsSaWeighReqstChannel::getNextAppRequestId();

            if (requestOutput_->publish(request)) {
                std::lock_guard<std::mutex> lck(mtx_);
                nextTxTimePoint_ = std::chrono::steady_clock::now() + timeoutDurationDefault();
                lastRequestId_ = request.appRequestId;
                lastCommand_ = request.command;
                success = true;
            }
        }

        return success;
    }

    /*
     * Wait for new tx data that reflects the changes made by the last request OR
     * a timeout has occurred.  If a timeout occurs, false is returned, otherwise true.
     * In any case, the txData is set to the most recent data that has been received.
     */
    bool waitForTxData(LpsSaWeighTxChannelStorage& txData, const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        std::unique_lock<std::mutex> lck(mtx_);

        // The data is new enough, which means:
        //  it is at least as new as the next expected tx time AND
        //  it is not initial data (has been recevied at least once)
        if ((txData_.timePoint >= nextTxTimePoint_) && (txData_.timePoint > timePointMin_)) {
            txData = txData_;
            success = true;
        }
        else {
            // Use a condition variable to wait for new data or a timeout
            // if it is a timeout, just use last data and return false
            if (std::cv_status::no_timeout == txDataCv_.wait_for(lck, timeoutDuration)) {
                success = true;
            }

            txData = txData_;
        }

        return success;
    }

    /*
     * Wait for a response to the most recent request.
     */
    bool waitForResponse(const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        std::unique_lock<std::mutex> lck(mtx_);
        if (LpsSaWeighReqstChannel::Command::NONE != lastCommand_) {
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
     * Send Request and Wait for Tx Data
     */
    bool sendRequestWaitForTxData(LpsSaWeighReqstChannel& request,
            LpsSaWeighTxChannelStorage& txData,
            const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = sendRequest(request);
        if (!waitForTxData(txData, timeoutDuration)) {
            success = false;
        }
        return success;
    }

    /*
     * Send a request and wait for the response
     */
    bool sendRequestGetResponse(LpsSaWeighReqstChannel& request,
            LpsSaWeighRespChannel& response,
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
    void getLastResponse(LpsSaWeighRespChannel& response) {
        std::lock_guard<std::mutex> lck(mtx_);
        response = responseData_;
    }

protected:
    void notifyResponseInput() {
        if (nullptr != responseInput_) {
            bool newData = false;
            LpsSaWeighRespChannel response;
            while (responseInput_->get(response)) {
                std::unique_lock<std::mutex> lck(mtx_);

                // Notify of the response
                responseData_ = response;

                if ((response.appName == appName_) &&
                        (response.appRequestId == lastRequestId_) &&
                        (response.command == lastCommand_)) {
                    lastCommand_ = LpsSaWeighReqstChannel::Command::NONE;
                    nextTxTimePoint_ = response.timePoint;
                    newData = true;
                    lck.unlock();
                    responseDataCv_.notify_all();
                }
            }

            if (newData) {
                std::unique_lock<std::mutex> lck(mtx_);
                if (txData_.timePoint >= nextTxTimePoint_) {
                    lck.unlock();
                    txDataCv_.notify_all();
                }
            }
        }
    }

    void notifyTxInput() {
        if (nullptr != txInput_) {
            bool newData = false;
            LpsSaWeighTxChannel txData;
            while (txInput_->get(txData)) {
                newData = true;
            }

            if (newData) {
                std::unique_lock<std::mutex> lck(mtx_);
                txData_ = txData;
                if (txData_.timePoint >= nextTxTimePoint_) {
                    lck.unlock();
                    txDataCv_.notify_all();
                }
            }
        }
    }

private:
    static constexpr auto timePointMin_ = std::chrono::steady_clock::time_point::min();

    std::string appName_;

    std::mutex mtx_;
    std::chrono::steady_clock::time_point nextTxTimePoint_;
    uint32_t lastRequestId_;
    std::condition_variable txDataCv_;
    LpsSaWeighTxChannelStorage txData_;

    LpsSaWeighReqstChannel::Command lastCommand_;
    std::condition_variable responseDataCv_;
    LpsSaWeighRespChannel responseData_;

    LpsSaWeighReqstChannelOutput* requestOutput_;
    LpsSaWeighRespChannelInput* responseInput_;
    LpsSaWeighTxChannelInput* txInput_;

    boost::signals2::connection responseInputConnection_;
    boost::signals2::connection txInputConnection_;
};

#endif
