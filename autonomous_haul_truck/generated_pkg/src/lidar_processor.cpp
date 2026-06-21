#include "lidar_processor.hpp"
#include <cmath>

using namespace std::chrono_literals;

LidarProcessor::LidarProcessor()
: Node("lidar_processor")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("scan_rate", 10.0);
    scan_rate_ = this->get_parameter("scan_rate").as_double();
    this->declare_parameter<double>("max_range", 100.0);
    max_range_ = this->get_parameter("max_range").as_double();
    this->declare_parameter<double>("obstacle_density", 0.15);
    obstacle_density_ = this->get_parameter("obstacle_density").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    scan_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        obstacle_map_pub_ = this->create_publisher<nav_msgs::msg::OccupancyGrid>("/truck/perception/obstacle_map", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /truck/perception/obstacle_map");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 10.0));
        scan_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&LidarProcessor::scan_tick, this),
            scan_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'scan_tick' started at 10.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void LidarProcessor::scan_tick()
{
    //-- begin impl [scan_tick] -----------------------------------------------
    ++tick_;
    auto grid = nav_msgs::msg::OccupancyGrid();
    grid.header.stamp    = this->now();
    grid.header.frame_id = "base_link";
    grid.info.resolution = 1.0f;
    grid.info.width      = static_cast<uint32_t>(grid_sz_);
    grid.info.height     = static_cast<uint32_t>(grid_sz_);
    grid.info.origin.position.x = 0.0;
    grid.info.origin.position.y = -(grid_sz_ / 2.0);
    grid.data.assign(grid_sz_ * grid_sz_, 0);

    // Simulate terrain obstacles using wave patterns (3 rock formations)
    double t  = tick_ * 0.1;
    int    cx = grid_sz_ / 2;
    for (int rock = 0; rock < 3; ++rock) {
        int rx = 20 + rock * 60 + static_cast<int>(std::sin(t * 0.3 + rock) * 8.0);
        int ry = cx + static_cast<int>(std::sin(t + rock * 2.1) * 25.0);
        for (int dy = -4; dy <= 4; ++dy)
            for (int dx = -4; dx <= 4; ++dx) {
                int x = rx + dx, y = ry + dy;
                if (x >= 0 && x < grid_sz_ && y >= 0 && y < grid_sz_)
                    grid.data[y * grid_sz_ + x] = 85;
            }
    }

    obstacle_map_pub_->publish(grid);
    //-- end impl [scan_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LidarProcessor>());
    rclcpp::shutdown();
    return 0;
}