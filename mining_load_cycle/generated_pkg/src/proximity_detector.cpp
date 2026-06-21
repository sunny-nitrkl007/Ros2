#include "proximity_detector.hpp"
#include <cmath>

using namespace std::chrono_literals;

ProximityDetector::ProximityDetector()
: Node("proximity_detector")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("check_rate", 10.0);
    check_rate_ = this->get_parameter("check_rate").as_double();
    this->declare_parameter<double>("proximity_radius", 12.0);
    proximity_radius_ = this->get_parameter("proximity_radius").as_double();
    this->declare_parameter<double>("excavator_x", 0.0);
    excavator_x_ = this->get_parameter("excavator_x").as_double();
    this->declare_parameter<double>("excavator_y", 0.0);
    excavator_y_ = this->get_parameter("excavator_y").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    prox_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        proximity_alert_pub_ = this->create_publisher<std_msgs::msg::Bool>("/mining/excavator/proximity_alert", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/excavator/proximity_alert");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = prox_group_;
        truck_pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mining/truck/truck_pose", qos,
            std::bind(&ProximityDetector::on_truck_pose, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/truck/truck_pose");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 10.0));
        proximity_check_timer_ = this->create_wall_timer(
            period,
            std::bind(&ProximityDetector::proximity_check, this),
            prox_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'proximity_check' started at 10.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ProximityDetector::on_truck_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
    //-- begin impl [on_truck_pose] ----------------------------------------
    truck_x_   = msg->pose.position.x;
    truck_y_   = msg->pose.position.y;
    has_truck_ = true;
    //-- end impl [on_truck_pose] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void ProximityDetector::proximity_check()
{
    //-- begin impl [proximity_check] -----------------------------------------------
    if (!has_truck_) return;

    double dx = truck_x_ - excavator_x_;
    double dy = truck_y_ - excavator_y_;
    double dist = std::sqrt(dx * dx + dy * dy);
    bool was_in = in_range_;
    in_range_ = (dist <= proximity_radius_);

    auto alert = std_msgs::msg::Bool();
    alert.data = in_range_;
    proximity_alert_pub_->publish(alert);

    if (in_range_ != was_in) {
        if (in_range_)
            RCLCPP_INFO(this->get_logger(), "Truck entered dig zone (%.1fm from excavator)", dist);
        else
            RCLCPP_INFO(this->get_logger(), "Truck left dig zone (%.1fm)", dist);
    }
    //-- end impl [proximity_check] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ProximityDetector>());
    rclcpp::shutdown();
    return 0;
}