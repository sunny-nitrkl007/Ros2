#include "watchdog.hpp"

using namespace std::chrono_literals;

Watchdog::Watchdog()
: Node("watchdog")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("heartbeat_rate", 10.0);
    heartbeat_rate_ = this->get_parameter("heartbeat_rate").as_double();
    this->declare_parameter<double>("timeout_threshold", 1.0);
    timeout_threshold_ = this->get_parameter("timeout_threshold").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    watch_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        heartbeat_pub_ = this->create_publisher<std_msgs::msg::Bool>("/adas/safety/heartbeat", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /adas/safety/heartbeat");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(10));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = watch_group_;
        safety_status_sub_ = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
            "/adas/safety/safety_status", qos,
            std::bind(&Watchdog::on_safety_status, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/safety/safety_status");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    system_health_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/adas/safety/system_health",
        std::bind(&Watchdog::on_system_health, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        watch_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /adas/safety/system_health");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 10.0));
        heartbeat_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&Watchdog::heartbeat_tick, this),
            watch_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'heartbeat_tick' started at 10.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Watchdog::on_safety_status(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg)
{
    //-- begin impl [on_safety_status] ----------------------------------------
    last_status_time_ = this->now();
    initialized_ = true;

    fault_count_ = 0;
    for (const auto & s : msg->status) {
        if (s.level >= diagnostic_msgs::msg::DiagnosticStatus::ERROR)
            ++fault_count_;
    }
    system_ok_ = (fault_count_ == 0);
    //-- end impl [on_safety_status] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void Watchdog::on_system_health(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_system_health] ----------------------------------------
    (void)request;
    response->success = system_ok_;
    response->message = system_ok_
        ? "System healthy"
        : "Faults detected: " + std::to_string(fault_count_);
    //-- end impl [on_system_health] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void Watchdog::heartbeat_tick()
{
    //-- begin impl [heartbeat_tick] ----------------------------------------
    auto hb = std_msgs::msg::Bool();
    hb.data = true;
    heartbeat_pub_->publish(hb);

    if (!initialized_) return;

    double age = (this->now() - last_status_time_).seconds();
    if (age > timeout_threshold_) {
        RCLCPP_ERROR(this->get_logger(),
            "safety_monitor timeout: last status %.2fs ago (threshold %.2fs)",
            age, timeout_threshold_);
        system_ok_ = false;
    } else if (!system_ok_) {
        RCLCPP_WARN(this->get_logger(), "System unhealthy — %d fault(s)", fault_count_);
    }
    //-- end impl [heartbeat_tick] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Watchdog>());
    rclcpp::shutdown();
    return 0;
}