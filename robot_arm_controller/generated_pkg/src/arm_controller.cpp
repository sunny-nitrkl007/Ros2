#include "arm_controller.hpp"

using namespace std::chrono_literals;

ArmController::ArmController()
: Node("arm_controller")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<std::vector<std::string>>("joint_names", {"shoulder", "elbow", "wrist"});
    joint_names_ = this->get_parameter("joint_names").as_string_array();
    this->declare_parameter<double>("max_velocity", 1.0);
    max_velocity_ = this->get_parameter("max_velocity").as_double();
    this->declare_parameter<double>("control_rate", 100.0);
    control_rate_ = this->get_parameter("control_rate").as_double();
    this->declare_parameter<std::string>("mode", "idle");
    mode_ = this->get_parameter("mode").as_string();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    control_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.best_effort();
        qos.deadline(std::chrono::milliseconds(20));
        joint_states_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("/robot1/joint_states", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /robot1/joint_states");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        qos.lifespan(std::chrono::milliseconds(1000));
        arm_status_pub_ = this->create_publisher<std_msgs::msg::String>("/robot1/arm_status", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /robot1/arm_status");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = control_group_;
        joint_commands_sub_ = this->create_subscription<trajectory_msgs::msg::JointTrajectory>(
            "/robot1/joint_commands", qos,
            std::bind(&ArmController::on_joint_commands, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /robot1/joint_commands");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    set_mode_srv_ = this->create_service<std_srvs::srv::SetBool>(
        "/robot1/set_mode",
        std::bind(&ArmController::on_set_mode, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        service_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /robot1/set_mode");

    // ── Service clients ───────────────────────────────────────────────────────
    check_collision_client_ = this->create_client<std_srvs::srv::Trigger>("/robot1/check_collision");
    RCLCPP_INFO(this->get_logger(), "Client created: /robot1/check_collision");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 100.0));
        control_loop_timer_ = this->create_wall_timer(
            period,
            std::bind(&ArmController::control_loop, this),
            control_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'control_loop' started at 100.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ArmController::on_joint_commands(const trajectory_msgs::msg::JointTrajectory::SharedPtr msg)
{
    //-- begin impl [on_joint_commands] ----------------------------------------
    if (msg->joint_names.empty()) return;
    RCLCPP_INFO(this->get_logger(), "Received joint_commands for %zu joints",
        msg->joint_names.size());
    //-- end impl [on_joint_commands] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void ArmController::on_set_mode(
    const std_srvs::srv::SetBool::Request::SharedPtr request,
    std_srvs::srv::SetBool::Response::SharedPtr response)
{
    //-- begin impl [on_set_mode] ----------------------------------------
    mode_ = request->data ? "active" : "idle";
    response->success = true;
    response->message = "Mode set to: " + mode_;
    RCLCPP_INFO(this->get_logger(), "Mode changed to: %s", mode_.c_str());
    //-- end impl [on_set_mode] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void ArmController::control_loop()
{
    //-- begin impl [control_loop] ----------------------------------------
    static uint64_t tick = 0;
    ++tick;

    // Publish sinusoidal joint states at 100 Hz
    auto js = sensor_msgs::msg::JointState();
    js.header.stamp = this->now();
    js.name = joint_names_;
    double t = tick * 0.01;
    for (size_t i = 0; i < joint_names_.size(); ++i) {
        double phase = static_cast<double>(i) * M_PI / 3.0;
        js.position.push_back(std::sin(t + phase) * max_velocity_);
        js.velocity.push_back(std::cos(t + phase) * max_velocity_);
    }
    joint_states_pub_->publish(js);

    // Publish arm_status every 1 s (100 ticks)
    if (tick % 100 == 0) {
        auto status = std_msgs::msg::String();
        status.data = "mode=" + mode_ + " tick=" + std::to_string(tick);
        arm_status_pub_->publish(status);
        RCLCPP_INFO(this->get_logger(), "Status: %s", status.data.c_str());
    }

    // Call check_collision every 2 s (200 ticks)
    if (tick % 200 == 0 && check_collision_client_->service_is_ready()) {
        auto req = std::make_shared<std_srvs::srv::Trigger::Request>();
        check_collision_client_->async_send_request(req,
            [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture future) {
                auto res = future.get();
                RCLCPP_INFO(this->get_logger(), "CollisionCheck: success=%d msg=%s",
                    res->success, res->message.c_str());
            });
    }
    //-- end impl [control_loop] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 4);
    auto node = std::make_shared<ArmController>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}