// Stage 1 direct-DDS test adapter -- JobMgr side.
// Development-Plan.txt Step 6.1.
//
// Builds the same 3 wrapper objects LpsSaJobMgrApp.cpp really builds for
// legs 1-3 (LpsSaWeighReqstChannel/RespChannel/TxChannel), on the exact
// same topic names, and hands them to a real `LpsSaWeighAppInf` instance
// -- the actual production class (Challenges-And-Decisions.txt 6.11),
// #include'd directly rather than mocked. Plus a 4th subscription for the
// hybrid LpsSaJobMgrReqstChannel leg (Step 6.1.4), and a 5th leg
// (Step 6.1 / 6.14) publishing LpsSaJobMgrTxChannel -- WeighApp only
// consumes 4 of its ~100 fields (standby_state, tip_off_state,
// tip_off_state_cfg, tip_off_trigger_type), which is exactly why the
// whole channel stays in job_mgr_interfaces rather than being wholesale-
// promoted like legs 1-4, but the leg itself is still genuinely direct
// DDS, so it's exercised here too. No AIS SDK anywhere in this file.

#include <chrono>
#include <memory>
#include <thread>

#include <rclcpp/rclcpp.hpp>

#include <ros_wrapper/RosInputInterface.h>
#include <ros_wrapper/RosOutputInterface.h>
#include <interfaces/LpsSaWeighReqstChannel/LpsSaWeighAppInf.hpp>

#include <cpm_common_interfaces/msg/lps_sa_job_mgr_reqst_channel.hpp>
#include <job_mgr_interfaces/msg/lps_sa_job_mgr_tx_channel.hpp>

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("jobmgr_adapter_node");

    // Same topic names LpsSaJobMgrApp.cpp/LpsSaWeighApp.cpp already agree on.
    auto* requestOutput = new ros_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>(
            node, "lps_sa_weigh_reqst_channel");
    auto* responseInput = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>(
            node, "lps_sa_weigh_resp_channel");
    auto* txInput = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>(
            node, "lps_sa_weigh_tx_channel");
    auto* jobMgrReqstIn = new ros_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaJobMgrReqstChannel>(
            node, "lps_sa_job_mgr_reqst_channel");

    // Leg 5 -- JobMgr publishes LpsSaJobMgrTxChannel; WeighApp genuinely
    // consumes 4 of its fields directly (see class comment above).
    auto* jobMgrTxOut = new ros_wrapper::RosOutputInterface<job_mgr_interfaces::msg::LpsSaJobMgrTxChannel>(
            node, "lps_sa_job_mgr_tx_channel");

    LpsSaWeighAppInf weighAppInf;
    if (!weighAppInf.start("jobmgr_adapter", requestOutput, responseInput, txInput)) {
        RCLCPP_ERROR(node->get_logger(), "weighAppInf.start() failed");
        return 1;
    }

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);

    RCLCPP_INFO(node->get_logger(), "jobmgr_adapter_node up -- 20ms tick, matching the real executive() rate");

    uint64_t tick = 0;
    bool standbyActivated = false;
    while (rclcpp::ok()) {
        // Real apps call spin_some() once at the top of executive(), before
        // touching any wrapper -- see RosInputInterface.h's design comment.
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

        // Leg 5 -- publish every tick, same cadence as the real 20ms
        // executive(). Toggle standby_state every ~2s (100 ticks) so the
        // WeighApp side can prove it's seeing live, changing data, not a
        // one-shot default. tip_off_state/tip_off_state_cfg/
        // tip_off_trigger_type set to distinct non-default values so each
        // field's identity is separately verifiable on the receiving end.
        if (tick % 100 == 0) {
            standbyActivated = !standbyActivated;
            RCLCPP_INFO(node->get_logger(), "standby_state toggled -> %s",
                    standbyActivated ? "ACTIVATED" : "DEACTIVATED");
        }
        job_mgr_interfaces::msg::LpsSaJobMgrTxChannel jobMgrTx;
        jobMgrTx.standby_state.value = standbyActivated
                ? cpm_common_interfaces::msg::StandbyState::ACTIVATED
                : cpm_common_interfaces::msg::StandbyState::DEACTIVATED;
        jobMgrTx.tip_off_state.value = cpm_common_interfaces::msg::TipOffState::TRUCK_ENABLE;
        jobMgrTx.tip_off_state_cfg.value = cpm_common_interfaces::msg::TipOffState::PILE_ENABLE;
        jobMgrTx.tip_off_trigger_type.value = cpm_common_interfaces::msg::TipOffTriggerType::MANUAL;
        jobMgrTxOut->publish(jobMgrTx);

        ++tick;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    rclcpp::shutdown();
    return 0;
}
