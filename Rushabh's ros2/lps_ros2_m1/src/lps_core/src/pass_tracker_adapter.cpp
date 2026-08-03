#include "lps_core/pass_tracker_adapter.hpp"
#include <cstring>
#include <utility>
extern "C" {
#include "LpsPtPublic.h"
}
namespace lps_core {
struct PassTrackerAdapter::Impl {
  LpsPtInputs_t input{};
  LpsPtOutputs_t output{};
  bool initialized{false};
};

PassTrackerAdapter::PassTrackerAdapter() : impl_(std::make_unique<Impl>()) {}
PassTrackerAdapter::~PassTrackerAdapter() = default;
PassTrackerAdapter::PassTrackerAdapter(PassTrackerAdapter &&) noexcept = default;
PassTrackerAdapter & PassTrackerAdapter::operator=(PassTrackerAdapter &&) noexcept = default;

void PassTrackerAdapter::initialize(uint8_t tipoff_mode) {
  LpsPtInit_t init{};
  init.tip_off_mode = tipoff_mode;
  impl_->output = LpsPtInit(&init);
  std::memset(&impl_->input, 0, sizeof(impl_->input));
  impl_->input.pt_time_step_us = 100000.0F;
  impl_->initialized = true;
}

PassTrackerOutput PassTrackerAdapter::step(const PassTrackerInput & in) {
  if (!impl_->initialized) initialize();
  auto & legacy = impl_->input;
  legacy.cpm_install_status = TRUE;
  legacy.cpm_is_calibrated = in.calibrated ? TRUE : FALSE;
  legacy.pt_time_step_us = 100000.0F;
  legacy.maximum_pass_count = 999;
  legacy.auto_store_enabled = FALSE;
  legacy.manual_add_enabled = FALSE;
  legacy.tip_off_trigger = TIP_OFF_TRIGGER_DISABLED;
  legacy.current_weight = in.current_weight;
  legacy.calc_method = in.calculation_method;
  legacy.current_bucket_weight_latched = in.bucket_weight_latched ? TRUE : FALSE;
  legacy.payload_latch_conditions_ok = in.latch_conditions_met ? TRUE : FALSE;
  legacy.lift_stalled = in.lift_stalled ? TRUE : FALSE;
  legacy.dig_detected = in.dig_status == LPS_WEIGHT_BKT_DIGGING;
  legacy.dump_state_is_partial_dump =
    in.dump_status == LPS_WEIGHT_BKT_PARTIALLY_DUMPED ||
    in.dump_status == LPS_WEIGHT_BKT_FULLY_DUMPED;
  legacy.dump_state_is_full_dump = in.dump_status == LPS_WEIGHT_BKT_FULLY_DUMPED;
  legacy.zero_request_status = in.zero_request ? TRUE : FALSE;
  legacy.minus_one_request_status = in.minus_one_request ? TRUE : FALSE;
  legacy.clear_request_status = in.clear_request ? TRUE : FALSE;
  legacy.store_request_status = in.store_request ? TRUE : FALSE;

  impl_->output = LpsPtUpdate(&legacy);
  const auto & o = impl_->output;
  PassTrackerOutput out;
  out.current_state = o.pt_current_state;
  out.standby_active = o.standby_active;
  out.show_clear_not_minus_one = o.show_clear_not_minus_one;
  out.tipoff_mode = o.tip_off_mode;
  out.tipoff_active = o.tip_off_active;
  out.configured_tipoff_mode = o.configured_tip_off_mode;
  out.manual_add_available = o.manual_add_available;
  out.pass_count = o.passcount;
  out.truck_weight = o.truck_weight;
  out.truck_start_weight = o.truck_start_weight;
  out.truck_pass_active = o.truck_pass_active;
  out.add_pass = o.loadUpdate.addPass;
  out.add_pass_weight = o.loadUpdate.addPassWeight;
  out.add_pass_accuracy = o.loadUpdate.addPassAccuracy;
  out.remove_pass = o.loadUpdate.removePass;
  out.clear = o.loadUpdate.clear;
  out.store = o.loadUpdate.store;
  out.auto_store = o.loadUpdate.isAutoStore;
  out.unlatch_current_bucket_weight = o.unlatch_current_bucket_weight;
  out.capture_cylinder_extension_reference = o.dump_detect_capture_cyl_ext_reference;
  out.clear_reweigh_warning = o.clear_reweigh_warning_status;
  out.display_bucket_weight = o.display_bucket_weight;
  out.display_bucket_weight_accuracy = o.display_bucket_weight_accuracy;
  out.display_bucket_weight_available = o.display_bucket_weight_available;

  legacy.cpm_is_calibrated = FALSE;
  legacy.dump_state_is_partial_dump = FALSE;
  legacy.dump_state_is_full_dump = FALSE;
  legacy.dig_detected = FALSE;
  legacy.store_request_status = FALSE;
  legacy.reweigh_request_status = FALSE;
  legacy.zero_request_status = FALSE;
  legacy.minus_one_request_status = FALSE;
  legacy.clear_request_status = FALSE;
  legacy.tipoff_toggle_request_status = FALSE;
  legacy.standby_request_status = FALSE;
  legacy.change_mode_weigh = FALSE;
  legacy.change_mode_excess = FALSE;
  legacy.manual_add_request = FALSE;
  return out;
}

void PassTrackerAdapter::restore_truck(float weight, uint16_t pass_count,
 bool have_last_pass, float last_pass_weight, uint8_t last_pass_accuracy) {
  LpsPtRestoreTruck(weight, pass_count, have_last_pass ? TRUE : FALSE,
                    last_pass_weight, last_pass_accuracy);
}

bool PassTrackerAdapter::initialized() const noexcept { return impl_->initialized; }
}  // namespace lps_core
