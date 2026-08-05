#ifndef LPSSAJOBMGRROSCHANNELS_H
#define LPSSAJOBMGRROSCHANNELS_H

#include <string>

#include <rclcpp/rclcpp.hpp>
#include <ros2_wrapper/RosInputInterface.h>
#include <ros2_wrapper/RosOutputInterface.h>

// DDSWeighAppInf, NOT LpsSaWeighAppInf -- that class is shared common/
// interfaces code, and at least one other real component
// (AisJhm2RequestProcessor, legacy UI infrastructure, still SCS-only)
// holds its own instance and depends on its original raw-SCS-typed API.
// DDSWeighAppInf is a new, separate class with only the 3 methods
// LpsSaJobMgrApp actually calls. See DDSWeighAppInf.hpp's own comment.
#include <interfaces/LpsSaWeighReqstChannel/DDSWeighAppInf.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_reqst_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_resp_channel.hpp>
#include <cpm_common_interfaces/msg/lps_sa_weigh_tx_channel.hpp>

///////////////////////////////////////////////////////////////////////////
// Stage 1 scope only (legs 1-3, the weighAppInf trio) -- same idea as the
// full-app RosChannels split discussed for the "everything" case, just
// narrowed to what this Stage 1 cut actually touches. Owns the ROS2 node,
// the executor, and weighAppInf. LpsSaJobMgrApp holds ONE instance of
// this instead of rosNode_/executor_/weighAppInf_ as 3 separate members.
///////////////////////////////////////////////////////////////////////////
class LpsSaJobMgrRosChannels {
public:
    LpsSaJobMgrRosChannels() = default;

    // rclcpp::init() must run once, before any Node is constructed -- this
    // app builds as its own standalone process (SConscript Program()
    // target, one task per process), so there's no risk of double-init
    // from another task sharing this process. One node for the whole app;
    // spinSome() must be called once per executive() tick so weighAppInf's
    // wrapper objects' callbacks fire -- see spinSome() below.
    bool init(const std::string& nodeName, const std::string& appName) {
        if (!rclcpp::ok()) {
            rclcpp::init(0, nullptr);
        }
        rosNode_ = std::make_shared<rclcpp::Node>(nodeName);

        // Topic names must match WeighApp's own construction of these 3
        // wrappers exactly (LpsSaWeighAppRosChannels.h on that side).
        auto* requestOutput = new ros2_wrapper::RosOutputInterface<cpm_common_interfaces::msg::LpsSaWeighReqstChannel>(
                rosNode_, "lps_sa_weigh_reqst_channel");
        auto* responseInput = new ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighRespChannel>(
                rosNode_, "lps_sa_weigh_resp_channel");
        auto* txInput = new ros2_wrapper::RosInputInterface<cpm_common_interfaces::msg::LpsSaWeighTxChannel>(
                rosNode_, "lps_sa_weigh_tx_channel");

        bool started = weighAppInf.start(appName, requestOutput, responseInput, txInput);

        executor_.add_node(rosNode_);

        return started;
    }

    // Drains pending callbacks for weighAppInf's 3 wrapper objects. Must
    // run before weighAppInf.waitForTxData()/sendRequest() -- same thread,
    // synchronous, no mutex needed (see RosInputInterface.h's design
    // comment for why).
    void spinSome() {
        executor_.spin_some();
    }

    void shutdown() {
        if (rclcpp::ok()) {
            rclcpp::shutdown();
        }
    }

    DDSWeighAppInf weighAppInf;

private:
    rclcpp::Node::SharedPtr rosNode_;
    rclcpp::executors::SingleThreadedExecutor executor_;
};

#endif
