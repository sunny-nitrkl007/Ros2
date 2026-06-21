#include "sensor_fusion.hpp"

using namespace std::chrono_literals;

SensorFusion::SensorFusion()
: Node("sensor_fusion")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("lidar_range", 50.0);
    lidar_range_ = this->get_parameter("lidar_range").as_double();
    this->declare_parameter<double>("fusion_rate", 50.0);
    fusion_rate_ = this->get_parameter("fusion_rate").as_double();
    this->declare_parameter<double>("grid_resolution", 0.1);
    grid_resolution_ = this->get_parameter("grid_resolution").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    fusion_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.best_effort();
        qos.deadline(std::chrono::milliseconds(20));
        fused_environment_pub_ = this->create_publisher<nav_msgs::msg::OccupancyGrid>("/adas/perception/fused_environment", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /adas/perception/fused_environment");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 50.0));
        fusion_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&SensorFusion::fusion_tick, this),
            fusion_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'fusion_tick' started at 50.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SensorFusion::fusion_tick()
{
    //-- begin impl [fusion_tick] ----------------------------------------
    ++tick_;

    auto grid = nav_msgs::msg::OccupancyGrid();
    grid.header.stamp    = this->now();
    grid.header.frame_id = "base_link";
    grid.info.resolution = grid_resolution_;
    grid.info.width      = static_cast<uint32_t>(grid_width_);
    grid.info.height     = static_cast<uint32_t>(grid_height_);
    grid.info.origin.position.x = -lidar_range_ / 2.0;
    grid.info.origin.position.y = -lidar_range_ / 2.0;

    grid.data.resize(grid_width_ * grid_height_, 0);

    // Simulate 2 moving obstacles using sinusoidal trajectories
    double t = tick_ * 0.02;
    auto mark_obstacle = [&](double wx, double wy, int radius) {
        int cx = static_cast<int>((wx - grid.info.origin.position.x) / grid_resolution_);
        int cy = static_cast<int>((wy - grid.info.origin.position.y) / grid_resolution_);
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int x = cx + dx, y = cy + dy;
                if (x >= 0 && x < grid_width_ && y >= 0 && y < grid_height_)
                    grid.data[y * grid_width_ + x] = 100;
            }
        }
    };

    mark_obstacle(std::sin(t) * 8.0,        5.0 + std::cos(t * 0.5) * 3.0, 3);
    mark_obstacle(std::cos(t * 0.7) * 6.0, -4.0 + std::sin(t * 0.3) * 2.0, 2);

    fused_environment_pub_->publish(grid);

    if (tick_ % 250 == 0)
        RCLCPP_INFO(this->get_logger(), "Fusion tick %lu — grid %dx%d published", tick_, grid_width_, grid_height_);
    //-- end impl [fusion_tick] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<SensorFusion>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}