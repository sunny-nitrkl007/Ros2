#include <chrono>
#include <cstdint>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "lps_core/latest_value_cache.hpp"
#include "lps_core/ordered_queue.hpp"
#include "lps_core/pass_tracker_adapter.hpp"
#include "lps_interfaces/msg/job_manager_state.hpp"
#include "lps_interfaces/msg/pass_tracker_debug.hpp"
#include "lps_interfaces/msg/weigh_state.hpp"
#include "lps_interfaces/srv/job_manager_command.hpp"
#include "lps_interfaces/srv/weigh_command.hpp"
using namespace std::chrono_literals;

class JobManagerNode final : public rclcpp::Node {
public:
  JobManagerNode() : Node("lps_job_manager"), command_queue_(20) 
  {
    auto qos=rclcpp::QoS(rclcpp::KeepLast(20)).reliable().durability_volatile();
    state_pub_=create_publisher<lps_interfaces::msg::JobManagerState>("/loader/job_manager/state",qos);
    debug_pub_=create_publisher<lps_interfaces::msg::PassTrackerDebug>("/loader/job_manager/pass_tracker_debug",qos);
    
    weigh_sub_=create_subscription<lps_interfaces::msg::WeighState>("/loader/weigh/state",qos,
      [this](lps_interfaces::msg::WeighState::ConstSharedPtr m){weigh_cache_.update(*m);});

    command_service_=create_service<lps_interfaces::srv::JobManagerCommand>(
      "/loader/job_manager/command",
      [this](const std::shared_ptr<lps_interfaces::srv::JobManagerCommand::Request> req,
             std::shared_ptr<lps_interfaces::srv::JobManagerCommand::Response> res){
        res->accepted=command_queue_.push(*req); res->success=res->accepted;
        res->detail=res->accepted?"queued for deterministic processing":"command queue full";
      });

    weigh_client_=create_client<lps_interfaces::srv::WeighCommand>("/loader/weigh/command");

    pass_tracker_.initialize();

    timer_=create_wall_timer(100ms,[this](){cycle();});

    RCLCPP_INFO(get_logger(),"Job Manager started with existing Pass Tracker C core and debug output");
  }
private:
  static uint64_t monotonic_ns(){return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
  void cycle(){
    auto weigh=weigh_cache_.snapshot();
    if(!weigh){
      if(!waiting_){
        RCLCPP_WARN(get_logger(),"Pass Tracker gated: waiting for first WeighState");waiting_=true;}
        publish_state(false);return;}
    waiting_=false;
    const bool fresh=weigh_cache_.is_fresh(350ms);

    if(!fresh){
      RCLCPP_WARN_THROTTLE(get_logger(),*get_clock(),2000,"WeighState is stale; Pass Tracker step skipped"); 
      publish_state(false); 
      publish_debug(false,*weigh); 
      return;
    }

    auto cmd=command_queue_.pop();

    lps_core::PassTrackerInput in;
    in.calibrated=weigh->calibration_status==0;
    in.dig_status=weigh->dig_status; in.dump_status=weigh->dump_status;
    in.current_weight=weigh->best_bucket_weight_tonnes;
    in.calculation_method=weigh->payload_calculation_method;
    in.bucket_weight_latched=weigh->bucket_weight_latched;
    in.latch_conditions_met=weigh->latch_conditions_met;
    in.lift_stalled=weigh->lift_stalled;

    if(cmd){
      in.zero_request=cmd->command==1;
      in.minus_one_request=cmd->command==2;
      in.clear_request=cmd->command==3;
      in.store_request=cmd->command==4;
    }

    output_=pass_tracker_.step(in);
    publish_state(true); publish_debug(true,*weigh);
  }
  void publish_state(bool fresh){
    lps_interfaces::msg::JobManagerState m; 
    m.stamp=now();
    m.source_monotonic_ns=monotonic_ns();
    m.sequence=state_sequence_++;
    m.task_number=1;

    m.operation_mode=fresh?0x0009:0x0346;
    m.pass_count=output_.pass_count;
    m.truck_weight=output_.truck_weight;
    m.truck_start_weight=output_.truck_start_weight;m.standby_state=output_.standby_active?0:1;m.tipoff_active=output_.tipoff_active;state_pub_->publish(m);
  }

  void publish_debug(bool fresh,const lps_interfaces::msg::WeighState & w){

    lps_interfaces::msg::PassTrackerDebug m;
    m.stamp=now();
    m.source_monotonic_ns=monotonic_ns();
    m.sequence=debug_sequence_++;
    m.pt_current_state=output_.current_state;
    m.standby_active=output_.standby_active;
    m.tipoff_active=output_.tipoff_active;
    m.tipoff_mode=output_.tipoff_mode;
    m.configured_tipoff_mode=output_.configured_tipoff_mode;
    m.show_clear_not_minus_one=output_.show_clear_not_minus_one;
    m.manual_add_available=output_.manual_add_available;
    m.pass_count=output_.pass_count;
    m.truck_weight=output_.truck_weight;
    m.truck_start_weight=output_.truck_start_weight;
    m.truck_pass_active=output_.truck_pass_active;
    m.add_pass=output_.add_pass;
    m.add_pass_weight=output_.add_pass_weight;
    m.add_pass_accuracy=output_.add_pass_accuracy;
    m.remove_pass=output_.remove_pass;
    m.clear=output_.clear;m.store=output_.store;
    m.auto_store=output_.auto_store;
    m.unlatch_current_bucket_weight=output_.unlatch_current_bucket_weight;
    m.capture_cylinder_extension_reference=output_.capture_cylinder_extension_reference;
    m.clear_reweigh_warning=output_.clear_reweigh_warning;
    m.display_bucket_weight=output_.display_bucket_weight;
    m.display_bucket_weight_accuracy=output_.display_bucket_weight_accuracy;
    m.display_bucket_weight_available=output_.display_bucket_weight_available;
    m.input_fresh=fresh;m.source_weigh_sequence=w.sequence;
    
    debug_pub_->publish(m);
  }
  lps_core::LatestValueCache<lps_interfaces::msg::WeighState> weigh_cache_;
  lps_core::OrderedQueue<lps_interfaces::srv::JobManagerCommand::Request> command_queue_;
  lps_core::PassTrackerAdapter pass_tracker_;
  lps_core::PassTrackerOutput output_;

  rclcpp::Publisher<lps_interfaces::msg::JobManagerState>::SharedPtr state_pub_;
  rclcpp::Publisher<lps_interfaces::msg::PassTrackerDebug>::SharedPtr debug_pub_;
  rclcpp::Subscription<lps_interfaces::msg::WeighState>::SharedPtr weigh_sub_;
  rclcpp::Service<lps_interfaces::srv::JobManagerCommand>::SharedPtr command_service_;
  rclcpp::Client<lps_interfaces::srv::WeighCommand>::SharedPtr weigh_client_;
  rclcpp::TimerBase::SharedPtr timer_;uint32_t state_sequence_{0},debug_sequence_{0};bool waiting_{false};
};
int main(int argc,char**argv){rclcpp::init(argc,argv);rclcpp::spin(std::make_shared<JobManagerNode>());rclcpp::shutdown();return 0;}
