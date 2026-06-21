#include "hazard_detector.hpp"
#include <cmath>
#include <string>

using namespace std::chrono_literals;

HazardDetector::HazardDetector()
: Node("hazard_detector")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("critical_distance", 8.0);
    critical_distance_ = this->get_parameter("critical_distance").as_double();
    this->declare_parameter<double>("warning_distance", 20.0);
    warning_distance_ = this->get_parameter("warning_distance").as_double();
    this->declare_parameter<double>("check_rate", 50.0);
    check_rate_ = this->get_parameter("check_rate").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    monitor_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(20));
        qos.lifespan(std::chrono::milliseconds(100));
        hazard_alert_pub_ = this->create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/truck/safety/hazard_alert", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /truck/safety/hazard_alert");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        obstacle_map_sub_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
            "/truck/perception/obstacle_map", qos,
            std::bind(&HazardDetector::on_obstacle_map, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /truck/perception/obstacle_map");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = monitor_group_;
        drive_command_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/truck/control/drive_command", qos,
            std::bind(&HazardDetector::on_drive_command, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /truck/control/drive_command");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    request_stop_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/truck/safety/request_stop",
        std::bind(&HazardDetector::on_request_stop, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        service_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /truck/safety/request_stop");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 50.0));
        hazard_check_timer_ = this->create_wall_timer(
            period,
            std::bind(&HazardDetector::hazard_check, this),
            monitor_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'hazard_check' started at 50.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void HazardDetector::on_obstacle_map(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
    //-- begin impl [on_obstacle_map] ----------------------------------------
    int W = static_cast<int>(msg->info.width);
    int H = static_cast<int>(msg->info.height);
    int cx = H / 2;
    nearest_dist_m_ = static_cast<double>(W) * msg->info.resolution;
    bool found = false;
    for (int x = 0; x < W && !found; ++x)
        for (int dy = -8; dy <= 8 && !found; ++dy) {
            int y = cx + dy;
            if (y >= 0 && y < H && msg->data[y * W + x] > 50) {
                nearest_dist_m_ = x * static_cast<double>(msg->info.resolution);
                found = true;
            }
        }
    //-- end impl [on_obstacle_map] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void HazardDetector::on_drive_command(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    //-- begin impl [on_drive_command] ----------------------------------------
    commanded_speed_ = msg->linear.x;
    //-- end impl [on_drive_command] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void HazardDetector::on_request_stop(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_request_stop] ----------------------------------------
    (void)request;
    emergency_active_ = true;
    response->success = true;
    response->message = "Emergency stop acknowledged by hazard_detector";
    RCLCPP_ERROR(this->get_logger(), "EMERGENCY STOP received — all motion halting");
    //-- end impl [on_request_stop] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void HazardDetector::hazard_check()
{
    //-- begin impl [hazard_check] -----------------------------------------------
    auto arr = diagnostic_msgs::msg::DiagnosticArray();
    arr.header.stamp = this->now();

    diagnostic_msgs::msg::DiagnosticStatus status;
    status.name = "proximity_hazard";

    if (emergency_active_) {
        status.level   = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
        status.message = "Emergency stop active — truck halted";
    } else if (nearest_dist_m_ < critical_distance_) {
        if (++consecutive_warn_ >= 5) {
            status.level   = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
            status.message = "CRITICAL obstacle at " + std::to_string(nearest_dist_m_).substr(0, 4) + "m";
        } else {
            status.level   = diagnostic_msgs::msg::DiagnosticStatus::WARN;
            status.message = "Obstacle approaching (" + std::to_string(nearest_dist_m_).substr(0, 4) + "m)";
        }
    } else if (nearest_dist_m_ < warning_distance_) {
        consecutive_warn_ = 0;
        status.level   = diagnostic_msgs::msg::DiagnosticStatus::WARN;
        status.message = "Obstacle in warning zone: " + std::to_string(nearest_dist_m_).substr(0, 5) + "m";
    } else {
        consecutive_warn_ = 0;
        status.level   = diagnostic_msgs::msg::DiagnosticStatus::OK;
        status.message = "Path clear (" + std::to_string(nearest_dist_m_).substr(0, 5) + "m)";
    }

    arr.status.push_back(status);
    hazard_alert_pub_->publish(arr);
    //-- end impl [hazard_check] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<HazardDetector>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}