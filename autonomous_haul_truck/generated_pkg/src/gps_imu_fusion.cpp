#include "gps_imu_fusion.hpp"
#include <cmath>

using namespace std::chrono_literals;

GpsImuFusion::GpsImuFusion()
: Node("gps_imu_fusion")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("fusion_rate", 20.0);
    fusion_rate_ = this->get_parameter("fusion_rate").as_double();
    this->declare_parameter<double>("position_noise", 0.3);
    position_noise_ = this->get_parameter("position_noise").as_double();
    this->declare_parameter<double>("heading_drift", 0.005);
    heading_drift_ = this->get_parameter("heading_drift").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    fusion_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.best_effort();
        qos.deadline(std::chrono::milliseconds(50));
        vehicle_pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/truck/localization/vehicle_pose", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /truck/localization/vehicle_pose");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 20.0));
        fusion_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&GpsImuFusion::fusion_tick, this),
            fusion_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'fusion_tick' started at 20.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void GpsImuFusion::fusion_tick()
{
    //-- begin impl [fusion_tick] -----------------------------------------------
    ++tick_;
    // Truck drives ~8 m/s toward destination_x = 500 m (reaches it in ~60 s simulation)
    pos_x_ += 0.40;
    heading_ += std::sin(tick_ * 0.02) * heading_drift_;

    auto msg = geometry_msgs::msg::PoseStamped();
    msg.header.stamp    = this->now();
    msg.header.frame_id = "map";
    msg.pose.position.x = pos_x_ + std::sin(tick_ * 0.07) * position_noise_;
    msg.pose.position.y = pos_y_ + std::cos(tick_ * 0.11) * position_noise_ * 0.3;
    msg.pose.orientation.z = std::sin(heading_ / 2.0);
    msg.pose.orientation.w = std::cos(heading_ / 2.0);

    vehicle_pose_pub_->publish(msg);

    if (tick_ % 100 == 0)
        RCLCPP_INFO(this->get_logger(), "GPS/IMU: pos=(%.1f, %.2f)  heading=%.4f rad",
            pos_x_, pos_y_, heading_);
    //-- end impl [fusion_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<GpsImuFusion>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}