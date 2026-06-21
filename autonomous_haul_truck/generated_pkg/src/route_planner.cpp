#include "route_planner.hpp"
#include <algorithm>
#include <cmath>

using namespace std::chrono_literals;

RoutePlanner::RoutePlanner()
: Node("route_planner")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("planning_rate", 5.0);
    planning_rate_ = this->get_parameter("planning_rate").as_double();
    this->declare_parameter<double>("destination_x", 500.0);
    destination_x_ = this->get_parameter("destination_x").as_double();
    this->declare_parameter<double>("destination_y", 0.0);
    destination_y_ = this->get_parameter("destination_y").as_double();
    this->declare_parameter<double>("waypoint_spacing", 20.0);
    waypoint_spacing_ = this->get_parameter("waypoint_spacing").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    planning_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        planned_route_pub_ = this->create_publisher<nav_msgs::msg::Path>("/truck/planning/planned_route", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /truck/planning/planned_route");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.best_effort();
        qos.deadline(std::chrono::milliseconds(50));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = planning_group_;
        vehicle_pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/truck/localization/vehicle_pose", qos,
            std::bind(&RoutePlanner::on_vehicle_pose, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /truck/localization/vehicle_pose");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = planning_group_;
        obstacle_map_sub_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
            "/truck/perception/obstacle_map", qos,
            std::bind(&RoutePlanner::on_obstacle_map, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /truck/perception/obstacle_map");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 5.0));
        planning_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&RoutePlanner::planning_tick, this),
            planning_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'planning_tick' started at 5.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void RoutePlanner::on_vehicle_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
    //-- begin impl [on_vehicle_pose] ----------------------------------------
    current_x_ = msg->pose.position.x;
    current_y_ = msg->pose.position.y;
    has_pose_  = true;
    //-- end impl [on_vehicle_pose] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void RoutePlanner::on_obstacle_map(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
    //-- begin impl [on_obstacle_map] ----------------------------------------
    // Scan forward column-by-column to find nearest obstacle in the truck's path
    int W = static_cast<int>(msg->info.width);
    int H = static_cast<int>(msg->info.height);
    int cx = H / 2;
    nearest_obs_m_ = W;
    bool found = false;
    for (int x = 5; x < W && !found; ++x)
        for (int dy = -7; dy <= 7 && !found; ++dy) {
            int y = cx + dy;
            if (y >= 0 && y < H && msg->data[y * W + x] > 50) {
                nearest_obs_m_ = x;
                found = true;
            }
        }
    //-- end impl [on_obstacle_map] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void RoutePlanner::planning_tick()
{
    //-- begin impl [planning_tick] -----------------------------------------------
    if (!has_pose_) return;

    auto path = nav_msgs::msg::Path();
    path.header.stamp    = this->now();
    path.header.frame_id = "map";

    double remaining = destination_x_ - current_x_;
    if (remaining < 5.0) {
        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
            "Destination reached at x=%.1fm", current_x_);
        planned_route_pub_->publish(path);
        return;
    }

    // Detour 9 m laterally if obstacle within 40 m
    bool   detour  = (nearest_obs_m_ < 40);
    double lateral = detour ? 9.0 : 0.0;
    int    steps   = 5;
    double look    = std::min(remaining, waypoint_spacing_ * steps);

    for (int i = 1; i <= steps; ++i) {
        geometry_msgs::msg::PoseStamped wp;
        wp.header = path.header;
        double r = static_cast<double>(i) / steps;
        wp.pose.position.x = current_x_ + look * r;
        wp.pose.position.y = current_y_ + lateral * (1.0 - r);
        wp.pose.orientation.w = 1.0;
        path.poses.push_back(wp);
    }
    planned_route_pub_->publish(path);

    if (detour)
        RCLCPP_INFO(this->get_logger(),
            "Obstacle at %dm — detouring %.0fm lateral, %.0fm to destination",
            nearest_obs_m_, lateral, remaining);
    //-- end impl [planning_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<RoutePlanner>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}