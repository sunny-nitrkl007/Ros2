// Stage 1 direct-DDS test harness -- JobMgr side.
// Development-Plan.txt Step 6.1 / Stage-Build-Test-Roadmap.txt 1.2a.
//
// Builds the same 3 shim objects LpsSaJobMgrApp.cpp really builds for
// legs 1-3 (LpsSaWeighReqstChannel/RespChannel/TxChannel), on the exact
// same topic names, and hands them to a real `LpsSaWeighAppInf` instance
// -- the actual production class (Challenges-And-Decisions.txt 6.11),
// #include'd directly rather than mocked. Plus a 4th subscription for the
// hybrid LpsSaJobMgrReqstChannel leg (Step 6.1.4). No AIS SDK anywhere in
// this file.

#include <chrono>
#include <memory>
#include <thread>

#include <rclcpp/rclcpp.hpp>

#include <ros_shim/RosInputInterface.h>
#include <ros_shim/RosOutputInterface.h>
#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp>

#include <cpm_common_interfaces/msg/lps_sa_job_mgr_reqst_channel.hpp>

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("jobmgr_harness_node");

    // Same topic names LpsSaJobMgrApp.cpp/LpsSaWeighApp.cpp already agree on.
    auto* requestOutput = new ros_shim::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>(
            node, "lps_sa_weigh_reqst_channel");
    auto* responseInput = new ros_shim::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>(
            node, "lps_sa_weigh_resp_channel");
    auto* txInput = new ros_shim::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>(
            node, "lps_sa_weigh_tx_channel");
    auto* jobMgrReqstIn = new ros_shim::RosInputInterface<cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel>(
            node, "lps_sa_job_mgr_reqst_channel");

    LpsSaWeighAppInf weighAppInf;
    if (!weighAppInf.start("jobmgr_harness", requestOutput, responseInput, txInput)) {
        RCLCPP_ERROR(node->get_logger(), "weighAppInf.start() failed");
        return 1;
    }

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);

    RCLCPP_INFO(node->get_logger(), "jobmgr_harness_node up -- 20ms tick, matching the real executive() rate");

    uint64_t tick = 0;
    while (rclcpp::ok()) {
        // Real apps call spin_some() once at the top of executive(), before
        // touching any shim -- see RosInputInterface.h's design comment.
        executor.spin_some();

        // Every ~1s (50 ticks @ 20ms), send a real command, same as
        // LpsSaJobMgrScsSendCmd() would on a real trigger.
        if (tick % 50 == 0) {
            cpm_common_interfaces::msg::LpsSaWeighReqstChannel request;
            request.command.value = cpm_common_interfaces::msg::WeighReqstChannelCommand::RESET_BEST_BUCKET_WEIGHT;
            bool sent = weighAppInf.sendRequest(request);
            RCLCPP_INFO(node->get_logger(), "sendRequest(RESET_BEST_BUCKET_WEIGHT) -> %s", sent ? "ok" : "FAILED");
        }

        // Same call LpsSaWeighScsTxParamRead() makes every tick.
        cpm_common_interfaces::msg::LpsSaWeighTxChannel txData;
        bool newData = weighAppInf.waitForTxData(txData);
        if (newData) {
            RCLCPP_INFO(node->get_logger(),
                    "tx data: dig_stat=%d cal_stat=%d dump_stat=%d best_bkt_wt_in_tonnes=%.3f lft_sealed=%s",
                    txData.dig_stat, txData.cal_stat, txData.dump_stat,
                    txData.best_bkt_wt_in_tonnes,
                    txData.lft_seal_status.sealed ? "true" : "false");
        }

        // Hybrid leg (6.1.4) -- drain whatever WeighApp's direct leg sent.
        cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel hybridMsg;
        while (jobMgrReqstIn->get(hybridMsg)) {
            RCLCPP_INFO(node->get_logger(), "hybrid leg: LpsSaJobMgrReqstChannel from app_name=%s app_request_id=%u",
                    hybridMsg.app_name.c_str(), hybridMsg.app_request_id);
        }

        ++tick;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    rclcpp::shutdown();
    return 0;
}
