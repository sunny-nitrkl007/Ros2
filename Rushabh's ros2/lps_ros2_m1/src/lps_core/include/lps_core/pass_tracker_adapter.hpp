#pragma once
#include <cstdint>
#include <memory>

namespace lps_core {
struct PassTrackerInput {
  bool calibrated{false};
  uint16_t dig_status{0};
  uint16_t dump_status{0};
  float current_weight{0.0F};
  uint16_t calculation_method{0};
  bool bucket_weight_latched{false};
  bool latch_conditions_met{false};
  bool lift_stalled{false};
  bool zero_request{false};
  bool minus_one_request{false};
  bool clear_request{false};
  bool store_request{false};
};

struct PassTrackerOutput {
  uint32_t current_state{0};
  bool standby_active{false};
  bool show_clear_not_minus_one{false};
  uint16_t tipoff_mode{0};
  bool tipoff_active{false};
  uint16_t configured_tipoff_mode{0};
  bool manual_add_available{false};

  uint16_t pass_count{0};
  float truck_weight{0.0F};
  float truck_start_weight{0.0F};
  bool truck_pass_active{false};

  bool add_pass{false};
  float add_pass_weight{0.0F};
  uint8_t add_pass_accuracy{0};
  bool remove_pass{false};
  bool clear{false};
  bool store{false};
  bool auto_store{false};

  bool unlatch_current_bucket_weight{false};
  bool capture_cylinder_extension_reference{false};
  bool clear_reweigh_warning{false};
  float display_bucket_weight{0.0F};
  uint8_t display_bucket_weight_accuracy{0};
  bool display_bucket_weight_available{false};
};

class PassTrackerAdapter {
public:
  PassTrackerAdapter();
  ~PassTrackerAdapter();
  PassTrackerAdapter(const PassTrackerAdapter &) = delete;
  PassTrackerAdapter & operator=(const PassTrackerAdapter &) = delete;
  PassTrackerAdapter(PassTrackerAdapter &&) noexcept;
  PassTrackerAdapter & operator=(PassTrackerAdapter &&) noexcept;

  void initialize(uint8_t tipoff_mode = 0);
  PassTrackerOutput step(const PassTrackerInput & input);
  void restore_truck(float weight, uint16_t pass_count, bool have_last_pass,
                     float last_pass_weight, uint8_t last_pass_accuracy);
  bool initialized() const noexcept;
private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace lps_core
