#ifndef DDSWEIGHAPPINFCB_HPP
#define DDSWEIGHAPPINFCB_HPP

#include <chrono>
#include <atomic>
#include <cstdint>
#include <limits>
#include <string>
#include <mutex>
#include <condition_variable>

#include <rclcpp/rclcpp.hpp>
#include <ros2_wrapper/RosInputInterfaceCb.h>
#include <ros2_wrapper/RosOutputInterface.h>

#include <cpm_common_interfaces/msg/lps_sa_weigh_reqst_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_resp_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_tx_channel.hpp>
#include <cpm_common_interfaces/msg/weigh_reqst_channel_command.hpp>

// PROTOTYPE -- not wired into any app. Pairs with RosInputInterfaceCb.h and
// LpsSaJobMgrRosChannelsCb.h to answer "can we get addNewDataSlot()-style
// callbacks and real blocking waits back?" This restores every method
// DDSWeighAppInf.hpp dropped (waitForResponse(), getLastResponse(),
// sendRequestGetResponse(), sendRequestWaitForTxData()) by pairing
// RosInputInterfaceCb<T>'s real callback with a mutex + condition_variable.
//
// This is structurally LpsSaWeighAppInf.hpp itself -- compare the two
// method-for-method, the shape is deliberately unchanged -- just retyped
// onto the new DDS message types (cpm_common_interfaces::msg::*) and
// RosInputInterfaceCb/RosOutputInterface instead of the real SCS
// InputInterface<T>/OutputInterface<T>. The mutex/condition_variable is
// back for the same reason it was there originally: notifyResponseInput()/
// notifyTxInput() now genuinely run on a different thread (the background
// spin thread owned by LpsSaJobMgrRosChannelsCb) than sendRequest()/
// waitForTxData()/waitForResponse(), so there is real concurrent access to
// guard -- unlike DDSWeighAppInf.hpp's poll-based design, where everything
// ran on one thread.
class DDSWeighAppInfCb {
public:
    static constexpr std::chrono::milliseconds timeoutDurationDefault() { return std::chrono::milliseconds(250); }

    DDSWeighAppInfCb() :
        appName_(),
        mtx_(),
        nextTxTimePointNs_(kTimePointMinNs),
        lastRequestId_(0),
        txDataCv_(),
        txData_(),
        lastCommand_(cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE),
        responseDataCv_(),
        responseData_(),
        requestOutput_(nullptr),
        responseInput_(nullptr),
        txInput_(nullptr) {

        // We have not received any data yet, so initialize this to be as stale as possible.
        txData_.time_point_ns = kTimePointMinNs;
    }

    virtual ~DDSWeighAppInfCb() {
        stop();
    }

    /* Start the interface service. Pointer overload only -- like
       DDSWeighAppInf, this is DDS-native, so there's no InterfaceDb
       string-bind overload to restore (the original's string-based start()
       was never reachable from LpsSaJobMgrApp anyway -- see chat
       discussion). */
    bool start(const std::string& appName,
            ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>* requestOutput,
            ros2_wrapper::RosInputInterfaceCb<cpm_common_interfaces::msg::LpsSaWeighRespChannel>* responseInput,
            ros2_wrapper::RosInputInterfaceCb<cpm_common_interfaces::msg::LpsSaWeighTxChannel>* txInput) {

        stop();

        // Reinitialize these if this is a "restart"
        nextTxTimePointNs_ = kTimePointMinNs;
        lastRequestId_ = 0;
        lastCommand_ = cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE;

        appName_ = appName;
        requestOutput_ = requestOutput;
        responseInput_ = responseInput;
        txInput_ = txInput;

        // Attach a listener to the responseInput channel -- direct
        // equivalent of LpsSaWeighAppInf.hpp's
        // responseInputConnection_ = responseInput_->addNewDataSlot(...).
        if (nullptr != responseInput_) {
            responseInput_->setNewDataCallback([this]{
                notifyResponseInput();
            });
        }

        // Attach a listener to the txInput channel.
        if (nullptr != txInput_) {
            txInput_->setNewDataCallback([this]{
                notifyTxInput();
            });
        }

        return (nullptr != requestOutput_) && (nullptr != responseInput_) && (nullptr != txInput_);
    }

    /* Stop the interface service. RosInputInterfaceCb has no connected()/
       disconnect() the way boost::signals2::connection did -- clearing the
       std::function is the direct equivalent. */
    bool stop() {
        if (nullptr != responseInput_) {
            responseInput_->setNewDataCallback(nullptr);
        }
        if (nullptr != txInput_) {
            txInput_->setNewDataCallback(nullptr);
        }
        return true;
    }

    /*
     * Send a request and set up to ignore incoming tx data until either
     * the request has been handled by the weigh app, or a timeout has elapsed.
     */
    bool sendRequest(cpm_common_interfaces::msg::LpsSaWeighReqstChannel& request) {
        bool success = false;

        if (nullptr != requestOutput_) {
            request.app_name = appName_;
            request.app_request_id = getNextAppRequestId();

            if (requestOutput_->publish(request)) {
                std::lock_guard<std::mutex> lck(mtx_);
                nextTxTimePointNs_ = nowNs() + std::chrono::duration_cast<std::chrono::nanoseconds>(timeoutDurationDefault()).count();
                lastRequestId_ = request.app_request_id;
                lastCommand_ = request.command.value;
                success = true;
            }
        }

        return success;
    }

    /*
     * Real blocking wait, restored: notifyTxInput() now genuinely runs on
     * a different thread (the background spin thread) and can wake this
     * one up via txDataCv_ -- same logic as LpsSaWeighAppInf.hpp's
     * waitForTxData().
     */
    bool waitForTxData(cpm_common_interfaces::msg::LpsSaWeighTxChannel& txData, const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        std::unique_lock<std::mutex> lck(mtx_);

        if ((txData_.time_point_ns >= nextTxTimePointNs_) && (txData_.time_point_ns > kTimePointMinNs)) {
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

    /*
     * Wait for a response to the most recent request.
     */
    bool waitForResponse(const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = false;
        std::unique_lock<std::mutex> lck(mtx_);
        if (cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE != lastCommand_) {
            if (std::cv_status::no_timeout == responseDataCv_.wait_for(lck, timeoutDuration)) {
                success = true;
            }
        }
        else {
            success = true;
        }

        return success;
    }

    bool sendRequestWaitForTxData(cpm_common_interfaces::msg::LpsSaWeighReqstChannel& request,
            cpm_common_interfaces::msg::LpsSaWeighTxChannel& txData,
            const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        bool success = sendRequest(request);
        if (!waitForTxData(txData, timeoutDuration)) {
            success = false;
        }
        return success;
    }

    bool sendRequestGetResponse(cpm_common_interfaces::msg::LpsSaWeighReqstChannel& request,
            cpm_common_interfaces::msg::LpsSaWeighRespChannel& response,
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

    void getLastResponse(cpm_common_interfaces::msg::LpsSaWeighRespChannel& response) {
        std::lock_guard<std::mutex> lck(mtx_);
        response = responseData_;
    }

protected:
    // Registered as responseInput_'s callback in start(). Runs on the
    // background spin thread -- direct equivalent of
    // LpsSaWeighAppInf.hpp's notifyResponseInput(). Body is unchanged
    // logic, just retyped onto the new message fields.
    void notifyResponseInput() {
        if (nullptr != responseInput_) {
            bool newData = false;
            cpm_common_interfaces::msg::LpsSaWeighRespChannel response;
            while (responseInput_->get(response)) {
                std::unique_lock<std::mutex> lck(mtx_);

                // Notify of the response
                responseData_ = response;

                if ((response.app_name == appName_) &&
                        (response.app_request_id == lastRequestId_) &&
                        (response.command.value == lastCommand_)) {
                    lastCommand_ = cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE;
                    nextTxTimePointNs_ = response.time_point_ns;
                    newData = true;
                    lck.unlock();
                    responseDataCv_.notify_all();
                }
            }

            if (newData) {
                std::unique_lock<std::mutex> lck(mtx_);
                if (txData_.time_point_ns >= nextTxTimePointNs_) {
                    lck.unlock();
                    txDataCv_.notify_all();
                }
            }
        }
    }

    // Equivalent of LpsSaWeighAppInf.hpp's notifyTxInput().
    void notifyTxInput() {
        if (nullptr != txInput_) {
            bool newData = false;
            cpm_common_interfaces::msg::LpsSaWeighTxChannel txData;
            while (txInput_->get(txData)) {
                newData = true;
            }

            if (newData) {
                std::unique_lock<std::mutex> lck(mtx_);
                txData_ = txData;
                if (txData_.time_point_ns >= nextTxTimePointNs_) {
                    lck.unlock();
                    txDataCv_.notify_all();
                }
            }
        }
    }

private:
    static constexpr int64_t kTimePointMinNs = std::numeric_limits<int64_t>::min();

    static int64_t nowNs() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static uint32_t getNextAppRequestId() {
        static std::atomic<uint32_t> nextAppRequestId(0);
        return nextAppRequestId++;
    }

    std::string appName_;

    std::mutex mtx_;
    int64_t nextTxTimePointNs_;
    uint32_t lastRequestId_;
    std::condition_variable txDataCv_;
    cpm_common_interfaces::msg::LpsSaWeighTxChannel txData_;

    uint8_t lastCommand_;
    std::condition_variable responseDataCv_;
    cpm_common_interfaces::msg::LpsSaWeighRespChannel responseData_;

    ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>* requestOutput_;
    ros2_wrapper::RosInputInterfaceCb<cpm_common_interfaces::msg::LpsSaWeighRespChannel>* responseInput_;
    ros2_wrapper::RosInputInterfaceCb<cpm_common_interfaces::msg::LpsSaWeighTxChannel>* txInput_;
};

#endif
