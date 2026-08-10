#ifndef DDSWEIGHAPPINF_HPP
#define DDSWEIGHAPPINF_HPP

#include <chrono>
#include <atomic>
#include <cstdint>
#include <limits>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <ros2_wrapper/RosInputInterface.h>
#include <ros2_wrapper/RosOutputInterface.h>

#include <cpm_common_interfaces/msg/lps_sa_weigh_reqst_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_resp_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_tx_channel.hpp>
#include <cpm_common_interfaces/msg/weigh_reqst_channel_command.hpp>

// New, separate class -- deliberately not a change to the shared
// LpsSaWeighAppInf header, which stays unchanged. Other components still
// hold their own instance of that class and call methods this app never
// needs, so editing it in place would break those callers. This class
// instead exposes only what's actually used: start(), sendRequest(),
// waitForTxData(). Threading note: RosInputInterface<T> is poll-only (no
// callback mechanism), so waitForTxData() is a single-shot poll of whatever
// the executor already delivered this tick, not a real blocking wait.
class DDSWeighAppInf {
public:
    static constexpr std::chrono::milliseconds timeoutDurationDefault() { return std::chrono::milliseconds(250); }

    DDSWeighAppInf() :
        appName_(),
        nextTxTimePointNs_(kTimePointMinNs),
        lastRequestId_(0),
        lastCommand_(cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE),
        txData_(),
        requestOutput_(nullptr),
        responseInput_(nullptr),
        txInput_(nullptr) {

        // We have not received any data yet, so initialize this to be as stale as possible.
        txData_.time_point_ns = kTimePointMinNs;
    }

    virtual ~DDSWeighAppInf() {
        stop();
    }

    /* Start the interface service. */
    bool start(const std::string& appName,
            ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>* requestOutput,
            ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>* responseInput,
            ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>* txInput) {

        stop();

        // Reinitialize these if this is a "restart"
        nextTxTimePointNs_ = kTimePointMinNs;
        lastRequestId_ = 0;
        lastCommand_ = cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE;

        appName_ = appName;
        requestOutput_ = requestOutput;
        responseInput_ = responseInput;
        txInput_ = txInput;

        return (nullptr != requestOutput_) && (nullptr != responseInput_) && (nullptr != txInput_);
    }

    /*
     * Stop the interface service. No connections to tear down under the
     * poll-based wrapper (nothing was subscribed to in the first place).
     */
    bool stop() {
        return true;
    }

    /*
     * Send a request.
     */
    bool sendRequest(cpm_common_interfaces::msg::LpsSaWeighReqstChannel& request) {
        bool success = false;

        if (nullptr != requestOutput_) {
            request.app_name = appName_;
            request.app_request_id = getNextAppRequestId();

            if (requestOutput_->publish(request)) {
                nextTxTimePointNs_ = nowNs() + std::chrono::duration_cast<std::chrono::nanoseconds>(timeoutDurationDefault()).count();
                lastRequestId_ = request.app_request_id;
                lastCommand_ = request.command.value;
                success = true;
            }
        }

        return success;
    }

    /*
     * Poll for tx data that reflects the changes made by the last request.
     * Drains both responseInput_ (to tighten the freshness gate once our
     * request's response arrives) and txInput_, then reports whether the
     * most recent tx data is new enough to reflect that response. Returns
     * false, same as a real timeout, if nothing new enough has arrived by
     * this tick -- there is no actual multi-tick wait to perform it.
     */
    bool waitForTxData(cpm_common_interfaces::msg::LpsSaWeighTxChannel& txData, const std::chrono::milliseconds& timeoutDuration = timeoutDurationDefault()) {
        (void)timeoutDuration; // retained for call-site compatibility; no real blocking wait is possible, see class comment

        drainResponseInput();
        drainTxInput();

        bool success = (txData_.time_point_ns >= nextTxTimePointNs_) && (txData_.time_point_ns > kTimePointMinNs);
        txData = txData_;

        return success;
    }

protected:
    void drainResponseInput() {
        if (nullptr != responseInput_) {
            cpm_common_interfaces::msg::LpsSaWeighRespChannel response;
            while (responseInput_->get(response)) {
                if ((response.app_name == appName_) &&
                        (response.app_request_id == lastRequestId_) &&
                        (response.command.value == lastCommand_)) {
                    lastCommand_ = cpm_common_interfaces::msg::WeighReqstChannelCommand::NONE;
                    nextTxTimePointNs_ = response.time_point_ns;
                }
            }
        }
    }

    void drainTxInput() {
        if (nullptr != txInput_) {
            cpm_common_interfaces::msg::LpsSaWeighTxChannel txData;
            while (txInput_->get(txData)) {
                txData_ = txData;
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

    int64_t nextTxTimePointNs_;
    uint32_t lastRequestId_;
    uint8_t lastCommand_;
    cpm_common_interfaces::msg::LpsSaWeighTxChannel txData_;

    ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>* requestOutput_;
    ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>* responseInput_;
    ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>* txInput_;
};

#endif
