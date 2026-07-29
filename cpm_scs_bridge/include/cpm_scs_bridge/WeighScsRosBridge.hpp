#pragma once

// AIS task framework
#include <task/Task.hpp>
#include <task/InterfaceDb.hpp>
#include <ais/interfaces/InputInterface/InputInterface.hpp>

// SCS channel types
#include <interfaces/LpsSaWeighTxChannel/LpsSaWeighTxChannel.h>
#include <interfaces/LpsSaJobMgrTxChannel/LpsSaJobMgrTxChannel.h>

// ROS2
#include <rclcpp/rclcpp.hpp>
#include <generated_interfaces/msg/weigh_status.hpp>

namespace cpm_scs_bridge
{

// Reads LpsSaWeighTxChannel from SCS every AIS executive tick and
// republishes its fields as a ROS2 WeighStatus message on /lps/weigh/status.
// WeighApp in AIS is NOT modified — this bridge is purely additive.
class WeighScsRosBridge : public task::Task
{
public:
    WeighScsRosBridge();
    ~WeighScsRosBridge() override = default;

    bool initialize() override;
    void executive() override;
    void cleanup() override;

private:
    generated_interfaces::msg::WeighStatus buildMsg(
        const LpsSaWeighTxChannelStorage & scs,
        const LpsSaJobMgrTxChannelStorage & jm) const;

    // SCS inputs — both already published by unmodified AIS apps
    InputInterface<LpsSaWeighTxChannel> * weighTxIn_  {nullptr};
    InputInterface<LpsSaJobMgrTxChannel>* jobMgrTxIn_ {nullptr};

    // ROS2 plumbing
    rclcpp::Node::SharedPtr             node_;
    rclcpp::executors::SingleThreadedExecutor executor_;
    rclcpp::Publisher<generated_interfaces::msg::WeighStatus>::SharedPtr pub_;

    // State for derivative computation (delta weight / dt)
    float prevWeight_  {0.0f};
    bool  firstTick_   {true};

    // AIS task rate in seconds — set by AIS task configuration (50 Hz → 0.02 s)
    static constexpr float kDtSec = 0.02f;
};

}  // namespace cpm_scs_bridge
