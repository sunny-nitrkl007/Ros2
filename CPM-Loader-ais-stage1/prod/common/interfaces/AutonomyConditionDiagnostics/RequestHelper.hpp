#ifndef AUTONOMYCONDITIONDIAGNOSTICS_REQUESTHELPER_HPP
#define AUTONOMYCONDITIONDIAGNOSTICS_REQUESTHELPER_HPP

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <ais/task/InterfaceDb.h>
#include <ais/log/Logger.h>

#include <boost/signals2.hpp>

#include <interfaces/AutonomyConditionDiagnostics/RequestInterface.hpp>
#include <interfaces/AutonomyConditionDiagnostics/RequestInterfaceOutputChannel.h>

#include <interfaces/AutonomyConditionDiagnostics/ResponseInterface.hpp>
#include <interfaces/AutonomyConditionDiagnostics/ResponseInterfaceInputChannel.h>

#include <interfaces/AutonomyConditionDiagnostics/TxInterfaceInputChannel.h>
#include <interfaces/AutonomyConditionDiagnostics/TxInterface.hpp>

class AutonomyConditionDiagnosticsRequestHelper {
public:
    static constexpr std::chrono::milliseconds timeoutDurationDefault() { return std::chrono::milliseconds(1000); }

    AutonomyConditionDiagnosticsRequestHelper() :
        appName_(),
        mtx_(),
        nextTxTimePoint_(timePointMin_),
        lastRequestId_(0),
        txDataCv_(),
        txData_(),
        requestOutput_(nullptr),
        responseInput_(nullptr),
        txInput_(nullptr),
        responseInputConnection_(),
        txInputConnection_() {

        // We have not received any data yet, so initialize this to be as stale as possible.
        txData_.timePoint = timePointMin_;
    }

    virtual ~AutonomyConditionDiagnosticsRequestHelper() {
        stop();
    }

    /*
     * Start the interface service.
     */
    bool start(const std::string& appName,
            const std::string& requestOutputName,
            const std::string& responseInputName,
            const std::string& txInputName) {
        bool everythingOk = true;

        AutonomyConditionDiagnosticsRequestInterfaceOutputChannel* requestOutput = nullptr;
        if (!task::InterfaceDb::bind(requestOutputName, requestOutput)) {
            AIS_LOG_ERROR("%s Interface not configured.", requestOutputName.c_str());
            everythingOk = false;
        }

        AutonomyConditionDiagnosticsResponseInterfaceInputChannel* responseInput = nullptr;
        if (!task::InterfaceDb::bind(responseInputName, responseInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", responseInputName.c_str());
            everythingOk = false;
        }

        AutonomyConditionDiagnosticsTxInterfaceInputChannel* txInput = nullptr;
        if (!task::InterfaceDb::bind(txInputName, txInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", txInputName.c_str());
            everythingOk = false;
        }

        if (everythingOk) {
            if (!start(appName, requestOutput, responseInput, txInput)) {
                AIS_LOG_ERROR("Failed to start autonomy condition request helper.");
                everythingOk = false;
            }
        }

        return everythingOk;
    }


    /*
     * Start the interface service.
     */
    bool start(const std::string& appName,
            AutonomyConditionDiagnosticsRequestInterfaceOutputChannel* requestOutput,
            AutonomyConditionDiagnosticsResponseInterfaceInputChannel* responseInput,
            AutonomyConditionDiagnosticsTxInterfaceInputChannel* txInput) {

        stop();

        nextTxTimePoint_ = timePointMin_;
        appName_ = appName;

        // Reinitialize these if this is a "restart"
        lastRequestId_ = 0;

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
    bool sendRequest(AutonomyConditionDiagnosticsRequestInterface& request) {
        bool success = false;

        if (nullptr != requestOutput_) {
            request.appName = appName_;
            request.appRequestId = AutonomyConditionDiagnosticsRequestInterface::getNextAppRequestId();

            if (requestOutput_->publish(request)) {
                std::lock_guard<std::mutex> lck(mtx_);
                lastRequestId_ = request.appRequestId;
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
        bool waitForTxData(AutonomyConditionDiagnosticsTxInterfaceStorage& txData, const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
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
         * Send Request and Wait for Tx Data
         */
        bool sendRequestWaitForTxData(AutonomyConditionDiagnosticsRequestInterface& request,
                AutonomyConditionDiagnosticsTxInterface& txData,
                const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
            bool success = sendRequest(request);
            if (!waitForTxData(txData, timeoutDuration)) {
                success = false;
            }
            return success;
        }


protected:
    void notifyResponseInput() {
        if (nullptr != responseInput_) {
            bool newData = false;
            AutonomyConditionDiagnosticsResponseInterface response;
            while (responseInput_->get(response)) {
                if (response.appName.empty()) {
                    // The app name is empty, this is just a broadcast response
                    // I just have to assume that the response is for me.
                    std::lock_guard<std::mutex> lck(mtx_);
                    nextTxTimePoint_ = response.timePoint;
                    newData = true;
                }
                else if (response.appName == appName_) {
                    std::lock_guard<std::mutex> lck(mtx_);
                    if (response.appRequestId == lastRequestId_) {
                        // This response is for me
                        nextTxTimePoint_ = response.timePoint;
                        newData = true;
                    }
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
            AutonomyConditionDiagnosticsTxInterface txData;
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
    AutonomyConditionDiagnosticsTxInterface txData_;

    AutonomyConditionDiagnosticsRequestInterfaceOutputChannel* requestOutput_;
    AutonomyConditionDiagnosticsResponseInterfaceInputChannel* responseInput_;
    AutonomyConditionDiagnosticsTxInterfaceInputChannel* txInput_;

    boost::signals2::connection responseInputConnection_;
    boost::signals2::connection txInputConnection_;


};

#endif /* AUTONOMYCONDITIONDIAGNOSTICS_REQUESTHELPER_HPP */
