#include "arm_planner.hpp"
#include <cmath>

using namespace std::chrono_literals;

ArmPlanner::ArmPlanner()
: Node("arm_planner")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("dig_rate", 5.0);
    dig_rate_ = this->get_parameter("dig_rate").as_double();
    this->declare_parameter<double>("dig_depth", 2.5);
    dig_depth_ = this->get_parameter("dig_depth").as_double();
    this->declare_parameter<double>("swing_speed", 0.3);
    swing_speed_ = this->get_parameter("swing_speed").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    arm_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        dig_command_pub_ = this->create_publisher<geometry_msgs::msg::Vector3>("/mining/excavator/dig_command", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/excavator/dig_command");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = arm_group_;
        proximity_alert_sub_ = this->create_subscription<std_msgs::msg::Bool>(
            "/mining/excavator/proximity_alert", qos,
            std::bind(&ArmPlanner::on_proximity_alert, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/excavator/proximity_alert");
    }

    // ── Service clients ───────────────────────────────────────────────────────
    request_load_client_ = this->create_client<std_srvs::srv::Trigger>("/mining/fleet/request_load");
    RCLCPP_INFO(this->get_logger(), "Client created: /mining/fleet/request_load");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 5.0));
        dig_cycle_timer_ = this->create_wall_timer(
            period,
            std::bind(&ArmPlanner::dig_cycle, this),
            arm_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'dig_cycle' started at 5.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ArmPlanner::on_proximity_alert(const std_msgs::msg::Bool::SharedPtr msg)
{
    //-- begin impl [on_proximity_alert] ----------------------------------------
    bool was_ready = truck_ready_;
    truck_ready_ = msg->data;
    if (truck_ready_ && !was_ready && arm_state_ == 0)
        RCLCPP_INFO(this->get_logger(), "Truck arrived at dig zone — starting excavation");
    //-- end impl [on_proximity_alert] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void ArmPlanner::dig_cycle()
{
    //-- begin impl [dig_cycle] -----------------------------------------------
    auto cmd = geometry_msgs::msg::Vector3();

    switch (arm_state_) {
        case 0:  // IDLE — wait for truck
            cmd.x = -90.0;
            if (truck_ready_) {
                arm_state_ = 1;
                dig_tick_  = 0;
                RCLCPP_INFO(this->get_logger(), "ARM: IDLE → DIGGING");
            }
            break;

        case 1:  // DIGGING — lower into ground
            ++dig_tick_;
            current_angle_ = -90.0 + 60.0 * std::min(1.0, dig_tick_ / 25.0);
            cmd.x = current_angle_;
            cmd.y = dig_depth_;
            cmd.z = swing_speed_;
            if (dig_tick_ >= 30) {
                arm_state_ = 2;
                RCLCPP_INFO(this->get_logger(), "ARM: DIGGING → RAISING");
            }
            break;

        case 2:  // RAISING — lift loaded bucket
            current_angle_ -= 3.0;
            cmd.x = current_angle_;
            if (current_angle_ <= -85.0) {
                arm_state_ = 3;
                dig_tick_  = 0;
                if (request_load_client_->service_is_ready()) {
                    auto req = std::make_shared<std_srvs::srv::Trigger::Request>();
                    request_load_client_->async_send_request(req,
                        [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture f) {
                            RCLCPP_INFO(this->get_logger(), "Dispatcher ack: %s", f.get()->message.c_str());
                        });
                }
                RCLCPP_INFO(this->get_logger(), "ARM: RAISING → DUMPING");
            }
            break;

        case 3:  // DUMPING — swing over truck and release
            ++dig_tick_;
            cmd.x = -20.0;  // swung over truck bed
            cmd.y = 0.0;
            if (dig_tick_ >= 20) {
                arm_state_    = 0;
                truck_ready_  = false;
                current_angle_ = -90.0;
                RCLCPP_INFO(this->get_logger(), "ARM: DUMPING → IDLE (cycle complete)");
            }
            break;
    }

    dig_command_pub_->publish(cmd);
    //-- end impl [dig_cycle] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<ArmPlanner>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}