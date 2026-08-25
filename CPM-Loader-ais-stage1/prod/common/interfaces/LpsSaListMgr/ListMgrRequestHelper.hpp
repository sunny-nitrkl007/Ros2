#ifndef LPSSALISTMGR_REQUESTHELPER_HPP
#define LPSSALISTMGR_REQUESTHELPER_HPP

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <ais/task/InterfaceDb.h>
#include <ais/log/Logger.h>
#include <boost/signals2.hpp>

#include <interfaces/LpsSaListMgr/InterfaceCommon.h>
#include <interfaces/LpsSaListMgr/NotifyInterface.hpp>
#include <interfaces/LpsSaListMgr/NotifyInterfaceInputChannel.h>

#include <interfaces/LpsSaListMgr/PostInterface.hpp>
#include <interfaces/LpsSaListMgr/PostInterfaceOutputChannel.h>

#include <interfaces/LpsSaListMgr/TxInterface.hpp>
#include <interfaces/LpsSaListMgr/TxInterfaceInputChannel.h>


class LpsSaListMgrRequestHelper {
public:

    static constexpr const std::chrono::milliseconds timeoutDurationDefault() { return std::chrono::milliseconds(2000); }

    LpsSaListMgrRequestHelper() :
        appName_(),
        mtx_(),
        lastRequestId_(0),
        nextTxTimePoint_(timePointMin_),
        txData_(),
        txDataCv_(),
        requestOutput_(nullptr),
        responseInput_(nullptr),
        txInput_(),
        responseInputConnection_(),
        txInputConnection_() {
        txData_.timePoint = timePointMin_;
    }

    virtual ~LpsSaListMgrRequestHelper() {
        stop();
    }

    /* Start the interface service */
    bool start(const std::string& appName, const std::string& requestOutputName, const std::string& responseInputName, const std::string& txInputName) {
        bool everythingOk = true;

        LpsSaListMgrPostInterfaceOutputChannel* requestOutput = nullptr;
        if (!task::InterfaceDb::bind(requestOutputName, requestOutput)) {
            AIS_LOG_ERROR("%s Interface not configured.", requestOutputName.c_str());
            everythingOk = false;
        }

        LpsSaListMgrNotifyInterfaceInputChannel* responseInput = nullptr;
        if (!task::InterfaceDb::bind(responseInputName, responseInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", responseInputName.c_str());
            everythingOk = false;
        }

        LpsSaListMgrTxInterfaceInputChannel* txInput;
        if (!task::InterfaceDb::bind(txInputName, txInput)) {
            AIS_LOG_ERROR("%s Interface not configured.", txInputName.c_str());
            everythingOk = false;
        }

        if (everythingOk) {
            if (!start(appName, requestOutput, responseInput, txInput)) {
                AIS_LOG_ERROR("Failed to start list manager request helper.");
                everythingOk = false;
            }
        }

        return everythingOk;
    }


    /* Start the interface service */
    bool start(const std::string& appName, LpsSaListMgrPostInterfaceOutputChannel* requestOutput,
            LpsSaListMgrNotifyInterfaceInputChannel* responseInput, LpsSaListMgrTxInterfaceInputChannel* txInput) {

        stop();

        // Reinitialize these if this is a "restart"
        lastRequestId_ = 0;
        appName_ = appName;
        nextTxTimePoint_ = timePointMin_;
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
        return (nullptr != requestOutput) && (responseInputConnection_.connected()) && (txInputConnection_.connected());
    }

    /* Stop the interface service */
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

    /* Send a request and set up to ignore incoming tx data until either
     * the request has been handled by the weigh app, or a timeout has elapsed */
    bool sendRequest(LpsSaListMgrPostInterface& request) {
        bool success = false;

        if (nullptr != requestOutput_) {
            request.appName = appName_;
            request.appRequestId = LpsSaListMgrPostInterface::getNextAppRequestId();

            if (requestOutput_->publish(request)) {
                std::lock_guard<std::mutex> lck(mtx_);
                lastRequestId_ = request.appRequestId;
                nextTxTimePoint_ = std::chrono::steady_clock::now() + timeoutDurationDefault();
                success = true;
            }
        }

        return success;
    }

    /* Wait for new tx data that reflects the changes made by the last request OR
     * a timeout has occurred.  If a timeout occurs, false is returned, otherwise true.
     * In any case, the txData is set to the most recent data that has been received */
    bool waitForTxData(LpsSaListMgrTxInterfaceStorage& txData, const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        std::unique_lock<std::mutex> lck(mtx_);
        if ((txData_.timePoint >= nextTxTimePoint_) && (txData_.timePoint > timePointMin_)) {
            txData = txData_;
            success = true;
        }
        else {
            if (std::cv_status::no_timeout == txDataCv_.wait_for(lck, timeoutDuration)) {
                success = true;
            }
            txData = txData_;
        }
        return success;
    }

    /* Send Request and Wait for Tx Data */
    bool sendRequestWaitForTxData(LpsSaListMgrPostInterface& request,
            LpsSaListMgrTxInterfaceStorage& txData,
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
            LpsSaListMgrNotifyInterface response;
            while (responseInput_->get(response)) {
                if (response.appName == appName_) {
                    std::unique_lock<std::mutex> lck(mtx_);
                    if (response.appRequestId == lastRequestId_) {
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
            LpsSaListMgrTxInterface txData;
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

    static constexpr const auto timePointMin_ = std::chrono::steady_clock::time_point::min();

    std::string appName_;
    std::mutex mtx_;
    uint32_t lastRequestId_;
    std::chrono::steady_clock::time_point nextTxTimePoint_;
    LpsSaListMgrTxInterfaceStorage txData_;
    std::condition_variable txDataCv_;
    LpsSaListMgrPostInterfaceOutputChannel* requestOutput_;
    LpsSaListMgrNotifyInterfaceInputChannel* responseInput_;
    LpsSaListMgrTxInterfaceInputChannel* txInput_;
    boost::signals2::connection responseInputConnection_;
    boost::signals2::connection txInputConnection_;

};

#endif
