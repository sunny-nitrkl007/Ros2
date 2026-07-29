#include "cpm_scs_bridge/WeighScsRosBridge.hpp"

#include <AIS_LOG.hpp>

namespace cpm_scs_bridge
{

// ──────────────────────────────────────────────────────────────────────────────
// Dump state values from LpsWeighBktDumpStat_t (defined in lps_weighing lib)
// Mirrored here so the bridge does not depend on the lps_weighing headers.
// ──────────────────────────────────────────────────────────────────────────────
static constexpr uint8_t DUMP_STATE_UNKNOWN            = 0;
static constexpr uint8_t DUMP_STATE_FULLY_RACKED       = 1;
static constexpr uint8_t DUMP_STATE_NOT_RACKED_DUMPED  = 2;
static constexpr uint8_t DUMP_STATE_PARTIALLY_DUMPED   = 3;
static constexpr uint8_t DUMP_STATE_FULLY_DUMPED       = 4;

// ──────────────────────────────────────────────────────────────────────────────

WeighScsRosBridge::WeighScsRosBridge()
: task::Task("WeighScsRosBridge")
{}

bool WeighScsRosBridge::initialize()
{
    bool ok = true;

    // ── Bind SCS inputs ──────────────────────────────────────────────────────
    // These channels are already published by unmodified WeighApp and JobMgr.
    if (!task::InterfaceDb::bind("LpsSaWeighTxChannelInput", weighTxIn_)) {
        AIS_LOG_ERROR("WeighScsRosBridge: failed to bind LpsSaWeighTxChannelInput");
        ok = false;
    }
    if (!task::InterfaceDb::bind("LpsSaJobMgrTxChannelInput", jobMgrTxIn_)) {
        AIS_LOG_WARN("WeighScsRosBridge: failed to bind LpsSaJobMgrTxChannelInput — "
                     "is_requested_gear_forward will default to false");
        // Not fatal — dig detection degrades gracefully without gear state
    }

    // ── ROS2 node and publisher ───────────────────────────────────────────────
    if (!rclcpp::ok()) {
        // rclcpp::init() is normally called once by the process entry point.
        // Guard here for unit-test environments that skip it.
        AIS_LOG_WARN("WeighScsRosBridge: rclcpp not initialised — calling init(0, nullptr)");
        rclcpp::init(0, nullptr);
    }

    rclcpp::NodeOptions opts;
    node_ = std::make_shared<rclcpp::Node>("weigh_scs_ros_bridge", opts);
    executor_.add_node(node_);

    auto qos = rclcpp::QoS(rclcpp::KeepLast(10))
                   .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
                   .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

    pub_ = node_->create_publisher<generated_interfaces::msg::WeighStatus>(
        "/lps/weigh/status", qos);

    AIS_LOG_INFO("WeighScsRosBridge: publishing WeighStatus on /lps/weigh/status");
    return ok;
}

void WeighScsRosBridge::executive()
{
    // ── Read SCS ─────────────────────────────────────────────────────────────
    LpsSaWeighTxChannelStorage scs;
    if (!weighTxIn_ || !weighTxIn_->get(scs)) {
        return;  // No new data from WeighApp this tick
    }

    LpsSaJobMgrTxChannelStorage jm;
    bool haveJobMgr = (jobMgrTxIn_ && jobMgrTxIn_->get(jm));
    (void)haveJobMgr;

    // ── Build and publish WeighStatus ─────────────────────────────────────────
    pub_->publish(buildMsg(scs, jm));

    // ── Drain any pending ROS2 callbacks (subscriptions added later) ──────────
    executor_.spin_some();
}

void WeighScsRosBridge::cleanup()
{
    pub_.reset();
    executor_.remove_node(node_);
    node_.reset();
}

// ──────────────────────────────────────────────────────────────────────────────
// Field-by-field mapping: LpsSaWeighTxChannelStorage → WeighStatus.msg
// ──────────────────────────────────────────────────────────────────────────────
generated_interfaces::msg::WeighStatus WeighScsRosBridge::buildMsg(
    const LpsSaWeighTxChannelStorage & scs,
    const LpsSaJobMgrTxChannelStorage & jm) const
{
    generated_interfaces::msg::WeighStatus msg;

    // ── Weight values ────────────────────────────────────────────────────────
    // BestBktWtInTonnes is the Butterworth-filtered best-available bucket weight.
    // The raw instantaneous weight is computed inside lps_weighing and not
    // separately exposed in the TxChannel, so we use the same value for both.
    msg.inst_weight_filtered = scs.BestBktWtInTonnes;
    msg.inst_weight_raw      = scs.BestBktWtInTonnes;
    msg.best_bucket_weight   = scs.BestBktWtInTonnes;

    // Weight derivative: rate of change per second, computed by the bridge.
    // On the first tick prevWeight_ is unset; derivative is zero.
    if (!firstTick_) {
        msg.inst_weight_derivative =
            (scs.BestBktWtInTonnes - prevWeight_) / kDtSec;
    }
    prevWeight_ = scs.BestBktWtInTonnes;
    firstTick_  = false;

    // Raw weight status: 0 = OK when calibrated
    msg.inst_weight_raw_status =
        (scs.CalStat == LPS_WEIGH_SYSTEM_CALIBRATED) ? 0u : 1u;

    // ── Pressure ─────────────────────────────────────────────────────────────
    // Head-end lift cylinder pressure, already in kPa inside ProdMeasureSensorStatus
    msg.lift_cyl_pres_compensated_kpa =
        scs.PidData.ProdMeasureSensorStatus.LiftCylHEPres;

    // ── Payload metadata ─────────────────────────────────────────────────────
    msg.payload_calc_method = scs.PayloadCalcMeth;
    msg.weigh_dec_flag      = scs.bktWtLatchedFlag;
    msg.latch_conditions_ok = scs.latchConditionsMet;

    // Weigh range indicator — cast from LpsWeighRangeIndicator_t enum
    msg.weigh_range_indicator = static_cast<uint16_t>(scs.Indicator);

    // ── Calibration / system state ────────────────────────────────────────────
    msg.cal_state_calibrated = (scs.CalStat == LPS_WEIGH_SYSTEM_CALIBRATED);
    msg.use_filter_data_flag = msg.cal_state_calibrated;

    // ── Dump state ────────────────────────────────────────────────────────────
    // DumpStat is LpsWeighBktDumpStat_t; values mirror the generated node's
    // numeric constants (0=UNKNOWN, 1=FULLY_RACKED, 4=FULLY_DUMPED).
    msg.dump_state       = static_cast<uint8_t>(scs.DumpStat);
    msg.full_rack_detected  = scs.BucketFullyRacked;
    msg.full_dump_detected  = (msg.dump_state == DUMP_STATE_FULLY_DUMPED);

    // ── Lift position ─────────────────────────────────────────────────────────
    // percentCylinderLength: 0% = fully retracted, 100% = fully extended
    msg.lift_cyl_extension_pct = scs.LiftPosition.percentCylinderLength;

    // Weigh range bottom is the start-of-weigh threshold in % cylinder extension
    msg.start_of_weigh_pct = scs.WeighRange.WeighRangeBottom;

    // ── Gear state ────────────────────────────────────────────────────────────
    // LpsSaWeighTxChannel does not carry gear state directly.
    // LpsSaJobMgrTxChannel carries TipOffState and StandbyState, but gear
    // comes from DataLinkData. For now we bind JobMgrTx and use its forward
    // indicator. TODO: bind DataLinkDataInput for authoritative gear signal.
    msg.is_requested_gear_forward = jm.IsRequestedGearForward;

    return msg;
}

}  // namespace cpm_scs_bridge
