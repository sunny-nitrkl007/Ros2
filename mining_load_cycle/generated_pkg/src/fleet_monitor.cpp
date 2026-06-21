#include "fleet_monitor.hpp"
#include <cmath>
#include <string>

using namespace std::chrono_literals;

FleetMonitor::FleetMonitor()
: Node("fleet_monitor")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("monitor_rate", 1.0);
    monitor_rate_ = this->get_parameter("monitor_rate").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    monitor_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(1000));
        health_telemetry_pub_ = this->create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/mining/fleet/health_telemetry", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/fleet/health_telemetry");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        bucket_state_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "/mining/excavator/bucket_state", qos,
            std::bind(&FleetMonitor::on_bucket_state, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/excavator/bucket_state");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        truck_pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mining/truck/truck_pose", qos,
            std::bind(&FleetMonitor::on_truck_pose, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/truck/truck_pose");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        payload_status_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "/mining/truck/payload_status", qos,
            std::bind(&FleetMonitor::on_payload_status, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/truck/payload_status");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    emergency_stop_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/mining/fleet/emergency_stop",
        std::bind(&FleetMonitor::on_emergency_stop, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        monitor_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /mining/fleet/emergency_stop");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 1.0));
        monitor_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&FleetMonitor::monitor_tick, this),
            monitor_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'monitor_tick' started at 1.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void FleetMonitor::on_bucket_state(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    //-- begin impl [on_bucket_state] ----------------------------------------
    last_bucket_load_ = msg->y;  // load_pct 0-1
    //-- end impl [on_bucket_state] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void FleetMonitor::on_truck_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
    //-- begin impl [on_truck_pose] ----------------------------------------
    last_truck_x_ = msg->pose.position.x;
    last_truck_y_ = msg->pose.position.y;
    //-- end impl [on_truck_pose] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void FleetMonitor::on_payload_status(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    //-- begin impl [on_payload_status] ----------------------------------------
    last_payload_pct_ = msg->z;
    //-- end impl [on_payload_status] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void FleetMonitor::on_emergency_stop(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_emergency_stop] ----------------------------------------
    (void)request;
    emergency_active_ = true;
    response->success = true;
    response->message = "Fleet emergency stop — all machines halting";
    RCLCPP_ERROR(this->get_logger(), "FLEET EMERGENCY STOP triggered");
    //-- end impl [on_emergency_stop] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void FleetMonitor::monitor_tick()
{
    //-- begin impl [monitor_tick] -----------------------------------------------
    auto arr = diagnostic_msgs::msg::DiagnosticArray();
    arr.header.stamp = this->now();

    auto add = [&](const std::string & name, int8_t lvl, const std::string & detail) {
        diagnostic_msgs::msg::DiagnosticStatus s;
        s.name = name; s.level = lvl; s.message = detail;
        arr.status.push_back(s);
    };

    int8_t em = emergency_active_ ? diagnostic_msgs::msg::DiagnosticStatus::ERROR
                                  : diagnostic_msgs::msg::DiagnosticStatus::OK;

    add("excavator.bucket", em,
        "load=" + std::to_string(static_cast<int>(last_bucket_load_ * 100)) + "%");

    double truck_dist = std::sqrt(last_truck_x_ * last_truck_x_ + last_truck_y_ * last_truck_y_);
    add("truck.position", em,
        "x=" + std::to_string(static_cast<int>(last_truck_x_)) +
        " y=" + std::to_string(static_cast<int>(last_truck_y_)) +
        " dist=" + std::to_string(static_cast<int>(truck_dist)) + "m");

    int8_t payload_lvl = (last_payload_pct_ >= 1.0)
        ? diagnostic_msgs::msg::DiagnosticStatus::WARN : em;
    add("truck.payload", payload_lvl,
        "load=" + std::to_string(static_cast<int>(last_payload_pct_ * 100)) + "%");

    health_telemetry_pub_->publish(arr);
    RCLCPP_INFO(this->get_logger(), "Fleet: bucket=%.0f%%  truck=(%.0f,%.0f)  payload=%.0f%%",
        last_bucket_load_ * 100, last_truck_x_, last_truck_y_, last_payload_pct_ * 100);
    //-- end impl [monitor_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<FleetMonitor>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}