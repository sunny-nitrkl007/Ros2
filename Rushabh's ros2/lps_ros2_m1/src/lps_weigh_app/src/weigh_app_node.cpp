#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "lps_core/latest_value_cache.hpp"
#include "lps_core/ordered_queue.hpp"
#include "lps_interfaces/msg/job_manager_state.hpp"
#include "lps_interfaces/msg/weigh_state.hpp"
#include "lps_interfaces/srv/job_manager_command.hpp"
#include "lps_interfaces/srv/weigh_command.hpp"

using namespace std::chrono_literals;

class WeighAppNode final : public rclcpp::Node {
public:
  WeighAppNode() : Node("lps_weigh_app"), command_queue_(20) {
    auto qos = rclcpp::QoS(rclcpp::KeepLast(20)).reliable().durability_volatile();

    state_publisher_ = create_publisher<lps_interfaces::msg::WeighState>(
      "/loader/weigh/state", qos);

    job_subscription_ = create_subscription<lps_interfaces::msg::JobManagerState>(
      "/loader/job_manager/state", qos,
      [this](lps_interfaces::msg::JobManagerState::ConstSharedPtr message) {
        job_state_cache_.update(*message);
      });

    command_service_ = create_service<lps_interfaces::srv::WeighCommand>(
      "/loader/weigh/command",
      [this](const std::shared_ptr<lps_interfaces::srv::WeighCommand::Request> request,
             std::shared_ptr<lps_interfaces::srv::WeighCommand::Response> response) {
        response->accepted = command_queue_.push(*request);
        response->success = response->accepted;
        response->detail = response->accepted ? "queued for deterministic processing" : "command queue full";
      });

    job_command_client_ = create_client<lps_interfaces::srv::JobManagerCommand>(
      "/loader/job_manager/command");

    processing_timer_ = create_wall_timer(20ms, [this]() { process_cycle(); });
    publication_timer_ = create_wall_timer(100ms, [this]() { publish_state(); });
    RCLCPP_INFO(get_logger(), "Weigh App started: 50 Hz processing, 10 Hz publication");
  }

private:
  void process_cycle() {
    // Milestone 1 deterministic mock signal. This block is replaced by the
    // existing lps_weighing and lps_cal calls in Milestone 3.
    scenario_tick_ = (scenario_tick_ + 1) % 250;

    // Stage 1: fully racked, unlatched and not digging.
    scenario_dig_status_ = 1;
    scenario_dump_status_ = 1;
    scenario_latched_ = false;
    scenario_latch_conditions_ = true;
    latest_weight_ = 2.5F;

    // Stage 2: digging edge before the bucket weight becomes available.
    if (scenario_tick_ >= 50 && scenario_tick_ < 75) {
      scenario_dig_status_ = 3;
    }

    // Stage 3: high-accuracy bucket weight becomes latched.
    if (scenario_tick_ >= 75 && scenario_tick_ < 125) {
      scenario_latched_ = true;
    }

    // Stage 4: partial dump closes the active pass.
    if (scenario_tick_ >= 125 && scenario_tick_ < 150) {
      scenario_dump_status_ = 3;
      scenario_latched_ = false;
    }

    // Stage 5: return fully racked and unlatched before next pass.
    if (scenario_tick_ >= 150) {
      scenario_dump_status_ = 1;
      scenario_latched_ = false;
    }

    if (auto command = command_queue_.pop()) {
      process_command(*command);
    }

    if (auto job_state = job_state_cache_.snapshot()) {
      latest_pass_count_ = job_state->pass_count;
    }
  }

  void process_command(const lps_interfaces::srv::WeighCommand::Request & command) {
    switch (command.command) {
      case lps_interfaces::srv::WeighCommand::Request::RESET_BEST_BUCKET_WEIGHT:
        latest_weight_ = 0.0F;
        break;
      case lps_interfaces::srv::WeighCommand::Request::CAPTURE_CYLINDER_EXTENSION_REFERENCE:
        cylinder_reference_captured_ = true;
        break;
      case lps_interfaces::srv::WeighCommand::Request::CLEAR_REWEIGH_WARNING:
        reweigh_warning_ = false;
        break;
      case lps_interfaces::srv::WeighCommand::Request::ZERO:
        latest_weight_ = 0.0F;
        notify_job_manager_zero();
        break;
      default:
        break;
    }
  }

  void notify_job_manager_zero() {
    if (!job_command_client_->service_is_ready()) {
      RCLCPP_WARN(get_logger(), "Job Manager command service is not ready");
      return;
    }
    auto request = std::make_shared<lps_interfaces::srv::JobManagerCommand::Request>();
    request->requester = get_name();
    request->legacy_request_id = next_request_id_++;
    request->command = lps_interfaces::srv::JobManagerCommand::Request::ZERO;
    job_command_client_->async_send_request(request);
  }

  void publish_state() {
    lps_interfaces::msg::WeighState message;
    message.stamp = now();
    message.source_monotonic_ns = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    message.sequence = sequence_++;
    message.dig_status = scenario_dig_status_;
    message.calibration_status = 0;
    message.dump_status = scenario_dump_status_;
    message.best_bucket_weight_tonnes = latest_weight_;
    message.payload_calculation_method = 769;
    message.bucket_weight_latched = scenario_latched_;
    message.latch_conditions_met = scenario_latch_conditions_;
    message.zero_available = true;
    message.zero_weight = 0.0F;
    message.simple_cal_adjust = 1.0F;
    message.lft_seal_status.sealed = false;
    message.lift_stalled = false;
    state_publisher_->publish(message);
  }

  lps_core::LatestValueCache<lps_interfaces::msg::JobManagerState> job_state_cache_;
  lps_core::OrderedQueue<lps_interfaces::srv::WeighCommand::Request> command_queue_;
  rclcpp::Publisher<lps_interfaces::msg::WeighState>::SharedPtr state_publisher_;
  rclcpp::Subscription<lps_interfaces::msg::JobManagerState>::SharedPtr job_subscription_;
  rclcpp::Service<lps_interfaces::srv::WeighCommand>::SharedPtr command_service_;
  rclcpp::Client<lps_interfaces::srv::JobManagerCommand>::SharedPtr job_command_client_;
  rclcpp::TimerBase::SharedPtr processing_timer_;
  rclcpp::TimerBase::SharedPtr publication_timer_;
  uint32_t sequence_{0};
  uint32_t next_request_id_{0};
  uint16_t latest_pass_count_{0};
  float latest_weight_{0.0F};
  double phase_{0.0};
  uint32_t scenario_tick_{0};
  uint16_t scenario_dig_status_{1};
  uint16_t scenario_dump_status_{1};
  bool scenario_latched_{false};
  bool scenario_latch_conditions_{true};
  bool cylinder_reference_captured_{false};
  bool reweigh_warning_{false};
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WeighAppNode>());
  rclcpp::shutdown();
  return 0;
}
