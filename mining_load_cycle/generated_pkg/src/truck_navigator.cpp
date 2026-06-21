#include "truck_navigator.hpp"
#include <algorithm>
#include <cmath>

using namespace std::chrono_literals;

TruckNavigator::TruckNavigator()
: Node("truck_navigator")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("nav_rate", 10.0);
    nav_rate_ = this->get_parameter("nav_rate").as_double();
    this->declare_parameter<double>("max_speed", 8.0);
    max_speed_ = this->get_parameter("max_speed").as_double();
    this->declare_parameter<double>("position_tolerance", 2.0);
    position_tolerance_ = this->get_parameter("position_tolerance").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    nav_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        truck_pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/mining/truck/truck_pose", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/truck/truck_pose");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = nav_group_;
        waypoint_command_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mining/fleet/waypoint_command", qos,
            std::bind(&TruckNavigator::on_waypoint_command, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/fleet/waypoint_command");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 10.0));
        navigation_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&TruckNavigator::navigation_tick, this),
            nav_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'navigation_tick' started at 10.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void TruckNavigator::on_waypoint_command(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
    //-- begin impl [on_waypoint_command] ----------------------------------------
    target_x_   = msg->pose.position.x;
    target_y_   = msg->pose.position.y;
    has_target_ = true;
    at_target_  = false;
    RCLCPP_INFO(this->get_logger(), "New waypoint: (%.1f, %.1f)", target_x_, target_y_);
    //-- end impl [on_waypoint_command] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void TruckNavigator::navigation_tick()
{
    //-- begin impl [navigation_tick] -----------------------------------------------
    auto pose = geometry_msgs::msg::PoseStamped();
    pose.header.stamp    = this->now();
    pose.header.frame_id = "map";
    pose.pose.orientation.w = 1.0;

    if (!has_target_ || at_target_) {
        pose.pose.position.x = current_x_;
        pose.pose.position.y = current_y_;
        truck_pose_pub_->publish(pose);
        return;
    }

    double dx   = target_x_ - current_x_;
    double dy   = target_y_ - current_y_;
    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= position_tolerance_) {
        at_target_ = true;
        current_x_ = target_x_;
        current_y_ = target_y_;
        RCLCPP_INFO(this->get_logger(), "Arrived at (%.1f, %.1f)", target_x_, target_y_);
    } else {
        double step = std::min(max_speed_ / 10.0, dist);  // 10 Hz
        current_x_ += (dx / dist) * step;
        current_y_ += (dy / dist) * step;
    }

    pose.pose.position.x = current_x_;
    pose.pose.position.y = current_y_;
    truck_pose_pub_->publish(pose);
    //-- end impl [navigation_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TruckNavigator>());
    rclcpp::shutdown();
    return 0;
}