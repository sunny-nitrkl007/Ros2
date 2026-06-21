#include "site_logger.hpp"
#include <string>

using namespace std::chrono_literals;

SiteLogger::SiteLogger()
: Node("site_logger")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("log_rate", 0.5);
    log_rate_ = this->get_parameter("log_rate").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    log_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(1000));
        cycle_report_pub_ = this->create_publisher<std_msgs::msg::String>("/mining/fleet/cycle_report", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/fleet/cycle_report");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = log_group_;
        payload_status_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "/mining/truck/payload_status", qos,
            std::bind(&SiteLogger::on_payload_status, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/truck/payload_status");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(1000));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = log_group_;
        health_telemetry_sub_ = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
            "/mining/fleet/health_telemetry", qos,
            std::bind(&SiteLogger::on_health_telemetry, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/fleet/health_telemetry");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 0.5));
        log_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&SiteLogger::log_tick, this),
            log_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'log_tick' started at 0.5 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SiteLogger::on_payload_status(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    //-- begin impl [on_payload_status] ----------------------------------------
    double pct = msg->z;
    // Detect cycle completion: payload resets from full back to near-zero
    if (last_payload_pct_ >= 0.95 && pct < 0.1) {
        ++cycle_count_;
        total_tonnes_ += msg->y / 1000.0;  // y = target_payload_kg → tonnes
        RCLCPP_INFO(this->get_logger(), "Load cycle %d complete — total hauled: %.1f t",
            cycle_count_, total_tonnes_);
    }
    last_payload_pct_ = pct;
    //-- end impl [on_payload_status] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SiteLogger::on_health_telemetry(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg)
{
    //-- begin impl [on_health_telemetry] ----------------------------------------
    fault_count_ = 0;
    for (const auto & s : msg->status)
        if (s.level >= diagnostic_msgs::msg::DiagnosticStatus::ERROR)
            ++fault_count_;
    //-- end impl [on_health_telemetry] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SiteLogger::log_tick()
{
    //-- begin impl [log_tick] -----------------------------------------------
    auto report = std_msgs::msg::String();
    report.data =
        "[SITE] cycles=" + std::to_string(cycle_count_) +
        " hauled=" + std::to_string(total_tonnes_).substr(0, 6) + "t" +
        " payload=" + std::to_string(static_cast<int>(last_payload_pct_ * 100)) + "%" +
        " faults=" + std::to_string(fault_count_);
    cycle_report_pub_->publish(report);
    RCLCPP_INFO(this->get_logger(), "%s", report.data.c_str());
    //-- end impl [log_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SiteLogger>());
    rclcpp::shutdown();
    return 0;
}