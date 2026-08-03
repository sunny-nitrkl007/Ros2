#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/set_bool.hpp"
#include "lps_interfaces/msg/weigh_state.hpp"
using namespace std::chrono_literals;

class ScenarioNode final : public rclcpp::Node {
public:

 ScenarioNode():Node("lps_pass_tracker_scenario"){

  auto q=rclcpp::QoS(rclcpp::KeepLast(20)).reliable().durability_volatile();
  
  pub_=create_publisher<lps_interfaces::msg::WeighState>("/loader/weigh/state",q);

  srv_=create_service<std_srvs::srv::SetBool>("/loader/scenario/enabled",
   [this](const std::shared_ptr<std_srvs::srv::SetBool::Request> req, std::shared_ptr<std_srvs::srv::SetBool::Response> res){enabled_=req->data;res->success=true;res->message=enabled_?"enabled":"paused";});
  timer_=create_wall_timer(100ms,[this](){tick();});
  RCLCPP_INFO(get_logger(),"Pass Tracker scenario publisher started at 10 Hz");
 }

private:
 void tick(){
    
    if(!enabled_)
        return;
        
    tick_=(tick_+1)%50;
    lps_interfaces::msg::WeighState m;

    m.stamp=now();
    m.sequence=seq_++;
    m.calibration_status=0;
    m.dig_status=1;
    m.dump_status=1;
    m.best_bucket_weight_tonnes=2.5F;
    m.payload_calculation_method=769;
    m.latch_conditions_met=true;
    m.zero_available=true;

    if(tick_>=10&&tick_<15)
        m.dig_status=3;
        
    if(tick_>=15&&tick_<25)
        m.bucket_weight_latched=true;

    
    if(tick_>=25&&tick_<30){
        m.dump_status=3;
        m.bucket_weight_latched=false;
    }
    pub_->publish(m);
 }

 rclcpp::Publisher<lps_interfaces::msg::WeighState>::SharedPtr pub_;
 rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr srv_;
 rclcpp::TimerBase::SharedPtr timer_;uint32_t tick_{0},seq_{0};bool enabled_{true};
};

int main(int c,char**v){
    rclcpp::init(c,v);
    rclcpp::spin(std::make_shared<ScenarioNode>());
    rclcpp::shutdown();
    
    return 0;
}
