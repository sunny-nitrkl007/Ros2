#include "truck_health_monitor.hpp"
#include <cmath>
#include <string>

using namespace std::chrono_literals;

TruckHealthMonitor::TruckHealthMonitor()
: Node("truck_health_monitor")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("engine_temp_max", 95.0);
    engine_temp_max_ = this->get_parameter("engine_temp_max").as_double();
    this->declare_parameter<double>("hydraulic_pressure_min", 150.0);
    hydraulic_pressure_min_ = this->get_parameter("hydraulic_pressure_min").as_double();
    this->declare_parameter<double>("report_rate", 2.0);
    report_rate_ = this->get_parameter("report_rate").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    health_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        health_report_pub_ = this->create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/truck/health/health_report", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /truck/health/health_report");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    system_status_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/truck/health/system_status",
        std::bind(&TruckHealthMonitor::on_system_status, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        health_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /truck/health/system_status");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 2.0));
        health_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&TruckHealthMonitor::health_tick, this),
            health_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'health_tick' started at 2.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void TruckHealthMonitor::on_system_status(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_system_status] ----------------------------------------
    (void)request;
    bool ok = (engine_temp_ <= engine_temp_max_) && (hydraulic_press_ >= hydraulic_pressure_min_);
    response->success = ok;
    response->message = ok ? "Truck systems nominal" :
        "Degraded — eng=" + std::to_string(static_cast<int>(engine_temp_)) + "C " +
        "hyd=" + std::to_string(static_cast<int>(hydraulic_press_)) + "bar";
    //-- end impl [on_system_status] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void TruckHealthMonitor::health_tick()
{
    //-- begin impl [health_tick] -----------------------------------------------
    ++tick_;
    // Simulate thermal cycle and hydraulic load variation
    engine_temp_    = 75.0 + 15.0 * std::sin(tick_ * 0.05) + std::sin(tick_ * 0.23) * 3.0;
    hydraulic_press_ = 172.0 - 18.0 * std::abs(std::sin(tick_ * 0.07));

    auto arr = diagnostic_msgs::msg::DiagnosticArray();
    arr.header.stamp = this->now();

    auto add_stat = [&](const std::string & name, int8_t level, const std::string & detail) {
        diagnostic_msgs::msg::DiagnosticStatus s;
        s.name = name; s.level = level; s.message = detail;
        arr.status.push_back(s);
    };

    add_stat("engine_temperature",
        (engine_temp_ > engine_temp_max_) ? diagnostic_msgs::msg::DiagnosticStatus::WARN
                                          : diagnostic_msgs::msg::DiagnosticStatus::OK,
        "temp=" + std::to_string(static_cast<int>(engine_temp_)) + "C");

    add_stat("hydraulic_pressure",
        (hydraulic_press_ < hydraulic_pressure_min_) ? diagnostic_msgs::msg::DiagnosticStatus::WARN
                                                     : diagnostic_msgs::msg::DiagnosticStatus::OK,
        "pressure=" + std::to_string(static_cast<int>(hydraulic_press_)) + "bar");

    health_report_pub_->publish(arr);

    if (tick_ % 4 == 0)
        RCLCPP_INFO(this->get_logger(), "Health: engine=%.0fC  hydraulic=%.0fbar",
            engine_temp_, hydraulic_press_);
    //-- end impl [health_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TruckHealthMonitor>());
    rclcpp::shutdown();
    return 0;
}