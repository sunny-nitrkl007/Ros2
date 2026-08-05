#ifndef LPSSAWEIGHAPPROSCHANNELS_H
#define LPSSAWEIGHAPPROSCHANNELS_H

#include <string>

#include <rclcpp/rclcpp.hpp>
#include <ros2_wrapper/RosInputInterface.h>
#include <ros2_wrapper/RosOutputInterface.h>

#include <cpm_common_interfaces/msg/lps_sa_weigh_reqst_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_resp_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_tx_channel.hpp>

///////////////////////////////////////////////////////////////////////////
// Stage 1 scope only (legs 1-3 -- LpsSaWeighScsReqstIn/RespOut/TxOut, the
// 3 wrappers LpsSaScs.cpp's LpsSaScsChkForReqst()/LpsSaScsSendReqstResponse()/
// LpsSaWeighingScsTx() use). Owns the ROS2 node, the executor, and the 3
// channel wrappers. LpsSaWeighApp holds ONE instance of this instead of
// rosNode_/executor_/3 pointers as separate members.
///////////////////////////////////////////////////////////////////////////
class LpsSaWeighAppRosChannels {
public:
    LpsSaWeighAppRosChannels() = default;

    // rclcpp::init() must run once, before any Node is constructed -- this
    // app builds as its own standalone process (SConscript Program()
    // target, one task per process), so there's no risk of double-init
    // from another task sharing this process. One node for the whole app;
    // spinSome() must be called once per executive() tick so these 3
    // wrappers' callbacks fire -- see spinSome() below.
    bool init(const std::string& nodeName) {
        if (!rclcpp::ok()) {
            rclcpp::init(0, nullptr);
        }
        rosNode_ = std::make_shared<rclcpp::Node>(nodeName);

        // Topic names must match JobMgr's own construction of these 3
        // wrappers exactly (LpsSaJobMgrRosChannels.h on that side).
        LpsSaWeighScsTxOut = new ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>(
                rosNode_, "lps_sa_weigh_tx_channel");
        LpsSaWeighScsRespOut = new ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>(
                rosNode_, "lps_sa_weigh_resp_channel");
        LpsSaWeighScsReqstIn = new ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>(
                rosNode_, "lps_sa_weigh_reqst_channel");

        executor_.add_node(rosNode_);

        return (nullptr != LpsSaWeighScsTxOut) && (nullptr != LpsSaWeighScsRespOut) && (nullptr != LpsSaWeighScsReqstIn);
    }

    // Drains pending callbacks for the 3 wrapper objects. Must run before
    // their get()/publish() call sites -- same thread, synchronous, no
    // mutex needed (see RosInputInterface.h's design comment for why).
    void spinSome() {
        executor_.spin_some();
    }

    void shutdown() {
        if (rclcpp::ok()) {
            rclcpp::shutdown();
        }
    }

    // Public on purpose -- LpsSaScs.cpp reads/writes these directly today
    // (e.g. LpsSaWeighScsReqstIn->get(...)). Keeping them public here means
    // those call sites only need a "rosChannels_." prefix added, nothing
    // about their own logic changes.
    ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>* LpsSaWeighScsReqstIn = nullptr;
    ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>* LpsSaWeighScsRespOut = nullptr;
    ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>* LpsSaWeighScsTxOut = nullptr;

private:
    rclcpp::Node::SharedPtr rosNode_;
    rclcpp::executors::SingleThreadedExecutor executor_;
};

#endif
