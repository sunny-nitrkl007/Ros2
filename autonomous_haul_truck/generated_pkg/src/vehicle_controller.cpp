#include "vehicle_controller.hpp"
#include <cmath>
#include <string>

using namespace std::chrono_literals;

VehicleController::VehicleController()
: Node("vehicle_controller")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("max_speed", 12.0);
    max_speed_ = this->get_parameter("max_speed").as_double();
    this->declare_parameter<double>("max_steering", 25.0);
    max_steering_ = this->get_parameter("max_steering").as_double();
    this->declare_parameter<double>("control_rate", 20.0);
    control_rate_ = this->get_parameter("control_rate").as_double();
    this->declare_parameter<double>("brake_decel", 2.5);
    brake_decel_ = this->get_parameter("brake_decel").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    control_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        drive_command_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/truck/control/drive_command", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /truck/control/drive_command");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = control_group_;
        planned_route_sub_ = this->create_subscription<nav_msgs::msg::Path>(
            "/truck/planning/planned_route", qos,
            std::bind(&VehicleController::on_planned_route, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /truck/planning/planned_route");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(20));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = control_group_;
        hazard_alert_sub_ = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
            "/truck/safety/hazard_alert", qos,
            std::bind(&VehicleController::on_hazard_alert, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /truck/safety/hazard_alert");
    }

    // ── Service clients ───────────────────────────────────────────────────────
    request_stop_client_ = this->create_client<std_srvs::srv::Trigger>("/truck/safety/request_stop");
    RCLCPP_INFO(this->get_logger(), "Client created: /truck/safety/request_stop");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 20.0));
        control_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&VehicleController::control_tick, this),
            control_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'control_tick' started at 20.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void VehicleController::on_planned_route(const nav_msgs::msg::Path::SharedPtr msg)
{
    //-- begin impl [on_planned_route] ----------------------------------------
    if (msg->poses.empty()) return;
    next_wp_x_ = msg->poses.front().pose.position.x;
    next_wp_y_ = msg->poses.front().pose.position.y;
    has_route_ = true;
    //-- end impl [on_planned_route] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void VehicleController::on_hazard_alert(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg)
{
    //-- begin impl [on_hazard_alert] ----------------------------------------
    bool prev_emergency = emergency_;
    hazard_level_ = 0;
    for (const auto & s : msg->status) {
        if (s.level == diagnostic_msgs::msg::DiagnosticStatus::ERROR) hazard_level_ = 2;
        else if (s.level == diagnostic_msgs::msg::DiagnosticStatus::WARN && hazard_level_ < 2)
            hazard_level_ = 1;
    }
    if (hazard_level_ == 2 && !prev_emergency) {
        emergency_ = true;
        RCLCPP_ERROR(this->get_logger(), "EMERGENCY: hazard at critical level — stopping");
        if (request_stop_client_->service_is_ready()) {
            auto req = std::make_shared<std_srvs::srv::Trigger::Request>();
            request_stop_client_->async_send_request(req,
                [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture f) {
                    RCLCPP_WARN(this->get_logger(), "Stop ack: %s", f.get()->message.c_str());
                    emergency_ = false;
                });
        }
    }
    //-- end impl [on_hazard_alert] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void VehicleController::control_tick()
{
    //-- begin impl [control_tick] -----------------------------------------------
    auto cmd = geometry_msgs::msg::Twist();

    if (emergency_ || !has_route_) {
        // Decelerate to stop
        current_speed_ = std::max(0.0, current_speed_ - brake_decel_ / control_rate_);
        cmd.linear.x = current_speed_;
        drive_command_pub_->publish(cmd);
        return;
    }

    double target_speed = (hazard_level_ == 1) ? max_speed_ * 0.4 : max_speed_;
    // Smooth acceleration
    current_speed_ += (target_speed - current_speed_) * 0.08;

    // Steer toward next waypoint (truck approximated at origin heading forward)
    double dx = next_wp_x_;
    double dy = next_wp_y_;
    double steering = 0.0;
    if (std::abs(dx) > 0.1)
        steering = std::atan2(dy, dx);

    constexpr double PI = 3.14159265358979323846;
    double steer_max = max_steering_ * PI / 180.0;
    steering = std::max(-steer_max, std::min(steer_max, steering));

    cmd.linear.x  = current_speed_;
    cmd.angular.z = steering;
    drive_command_pub_->publish(cmd);
    //-- end impl [control_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<VehicleController>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}