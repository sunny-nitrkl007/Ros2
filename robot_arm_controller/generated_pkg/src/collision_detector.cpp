#include "collision_detector.hpp"

using namespace std::chrono_literals;

CollisionDetector::CollisionDetector()
: Node("collision_detector")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("safety_margin", 0.05);
    safety_margin_ = this->get_parameter("safety_margin").as_double();
    this->declare_parameter<double>("check_rate", 50.0);
    check_rate_ = this->get_parameter("check_rate").as_double();
    this->declare_parameter<bool>("alert_on_proximity", true);
    alert_on_proximity_ = this->get_parameter("alert_on_proximity").as_bool();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    safety_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        qos.lifespan(std::chrono::milliseconds(1000));
        collision_alert_pub_ = this->create_publisher<std_msgs::msg::Bool>("/robot1/collision_alert", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /robot1/collision_alert");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.best_effort();
        qos.deadline(std::chrono::milliseconds(20));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = safety_group_;
        joint_states_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "/robot1/joint_states", qos,
            std::bind(&CollisionDetector::on_joint_states, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /robot1/joint_states");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    check_collision_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/robot1/check_collision",
        std::bind(&CollisionDetector::on_check_collision, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        safety_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /robot1/check_collision");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 50.0));
        safety_check_timer_ = this->create_wall_timer(
            period,
            std::bind(&CollisionDetector::safety_check, this),
            safety_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'safety_check' started at 50.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void CollisionDetector::on_joint_states(const sensor_msgs::msg::JointState::SharedPtr msg)
{
    //-- begin impl [on_joint_states] ----------------------------------------
    latest_positions_ = msg->position;
    //-- end impl [on_joint_states] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void CollisionDetector::on_check_collision(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_check_collision] ----------------------------------------
    (void)request;
    response->success = !collision_detected_;
    response->message = collision_detected_ ? "COLLISION DETECTED" : "Clear";
    //-- end impl [on_check_collision] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void CollisionDetector::safety_check()
{
    //-- begin impl [safety_check] ----------------------------------------
    if (latest_positions_.empty()) return;

    bool any_collision = false;
    for (double pos : latest_positions_) {
        if (std::abs(pos) > (1.0 - safety_margin_)) {
            any_collision = true;
            break;
        }
    }

    if (any_collision != collision_detected_) {
        collision_detected_ = any_collision;
        auto alert = std_msgs::msg::Bool();
        alert.data = collision_detected_;
        collision_alert_pub_->publish(alert);
        RCLCPP_WARN(this->get_logger(), "Collision state changed: %s",
            collision_detected_ ? "COLLISION" : "Clear");
    }
    //-- end impl [safety_check] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<CollisionDetector>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}