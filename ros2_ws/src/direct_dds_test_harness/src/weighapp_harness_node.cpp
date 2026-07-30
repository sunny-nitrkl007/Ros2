// Stage 1 direct-DDS test harness -- WeighApp side.
// Development-Plan.txt Step 6.1 / Stage-Build-Test-Roadmap.txt 1.2b.
//
// Mirror-image of jobmgr_harness_node: builds the 4 shims WeighApp's real
// code builds for these legs (LpsSaScs.cpp/LpsSaWeighApp.cpp), on the same
// topic names. NOT the real business logic (that needs the full AIS-linked
// build, see Stage-Build-Test-Roadmap.txt 1.5) -- a canned responder that
// echoes requests and publishes plausible tx heartbeats, just enough to
// exercise the wire format and the real LpsSaWeighAppInf logic on the
// other end. No AIS SDK anywhere in this file.

#include <chrono>
#include <memory>
#include <thread>

#include <rclcpp/rclcpp.hpp>

#include <ros_shim/RosInputInterface.h>
#include <ros_shim/RosOutputInterface.h>

#include <cpm_common_interfaces/msg/lps_sa_weigh_reqst_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_resp_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_tx_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_job_mgr_reqst_channel.hpp>

namespace {

int64_t nowNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

} // namespace

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("weighapp_harness_node");

    auto* reqstIn = new ros_shim::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>(
            node, "lps_sa_weigh_reqst_channel");
    auto* respOut = new ros_shim::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>(
            node, "lps_sa_weigh_resp_channel");
    auto* txOut = new ros_shim::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>(
            node, "lps_sa_weigh_tx_channel");
    auto* jobMgrReqstOut = new ros_shim::RosOutputInterface<cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel>(
            node, "lps_sa_job_mgr_reqst_channel");

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);

    RCLCPP_INFO(node->get_logger(), "weighapp_harness_node up -- 20ms tick, canned responder");

    uint64_t tick = 0;
    bool sealed = false;

    while (rclcpp::ok()) {
        executor.spin_some();

        // Drain requests, mirrors LpsSaScsSendReqstResponse's real overload-3
        // shape (Development-Plan.txt Step 5.2 / LpsSaScs.cpp:432).
        cpm_common_interfaces::msg::LpsSaWeighReqstChannel request;
        while (reqstIn->get(request)) {
            RCLCPP_INFO(node->get_logger(), "received request: app_name=%s app_request_id=%u command=%u",
                    request.app_name.c_str(), request.app_request_id,
                    static_cast<unsigned int>(request.command.value));

            cpm_common_interfaces::msg::LpsSaWeighRespChannel response;
            response.app_name = request.app_name;
            response.app_request_id = request.app_request_id;
            response.time_point_ns = nowNs();
            response.command = request.command;
            response.success = true;
            respOut->publish(response);
            RCLCPP_INFO(node->get_logger(), "published response: app_request_id=%u success=true",
                    response.app_request_id);
        }

        // Tx heartbeat every tick, same cadence as the real 20ms executive().
        cpm_common_interfaces::msg::LpsSaWeighTxChannel txData;
        txData.time_point_ns = nowNs();
        txData.best_bkt_wt_in_tonnes = 12.5f;
        txData.lft_seal_status.sealed = sealed;
        txOut->publish(txData);

        // Toggle the seal flag every ~2s (100 ticks) to exercise JobMgr's
        // seal-detection branch (LpsSaJobMgrScs.cpp:895-899).
        if (tick % 100 == 0) {
            sealed = !sealed;
            RCLCPP_INFO(node->get_logger(), "lft_seal_status.sealed toggled -> %s", sealed ? "true" : "false");
        }

        // Hybrid leg (6.1.4) -- publish a canned request every ~5s (250 ticks).
        if (tick % 250 == 0) {
            cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel hybridMsg;
            hybridMsg.app_name = "weighapp_harness";
            hybridMsg.app_request_id = static_cast<uint32_t>(tick);
            jobMgrReqstOut->publish(hybridMsg);
            RCLCPP_INFO(node->get_logger(), "published hybrid leg LpsSaJobMgrReqstChannel app_request_id=%llu",
                    static_cast<unsigned long long>(tick));
        }

        ++tick;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    rclcpp::shutdown();
    return 0;
}
