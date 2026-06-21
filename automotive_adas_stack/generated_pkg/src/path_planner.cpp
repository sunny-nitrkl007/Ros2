#include "path_planner.hpp"

using namespace std::chrono_literals;

PathPlanner::PathPlanner()
: Node("path_planner")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("planning_rate", 20.0);
    planning_rate_ = this->get_parameter("planning_rate").as_double();
    this->declare_parameter<double>("look_ahead_distance", 10.0);
    look_ahead_distance_ = this->get_parameter("look_ahead_distance").as_double();
    this->declare_parameter<double>("max_speed", 15.0);
    max_speed_ = this->get_parameter("max_speed").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    planning_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        planned_path_pub_ = this->create_publisher<nav_msgs::msg::Path>("/adas/planning/planned_path", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /adas/planning/planned_path");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = planning_group_;
        detected_objects_sub_ = this->create_subscription<visualization_msgs::msg::MarkerArray>(
            "/adas/perception/detected_objects", qos,
            std::bind(&PathPlanner::on_detected_objects, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/perception/detected_objects");
    }

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 20.0));
        planning_cycle_timer_ = this->create_wall_timer(
            period,
            std::bind(&PathPlanner::planning_cycle, this),
            planning_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'planning_cycle' started at 20.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void PathPlanner::on_detected_objects(const visualization_msgs::msg::MarkerArray::SharedPtr msg)
{
    //-- begin impl [on_detected_objects] ----------------------------------------
    nearest_object_count_ = static_cast<int>(msg->markers.size());
    nearest_obstacle_x_ = 999.0;
    for (const auto & m : msg->markers) {
        double dist = std::sqrt(m.pose.position.x * m.pose.position.x +
                                m.pose.position.y * m.pose.position.y);
        if (dist < nearest_obstacle_x_)
            nearest_obstacle_x_ = dist;
    }
    //-- end impl [on_detected_objects] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void PathPlanner::planning_cycle()
{
    //-- begin impl [planning_cycle] ----------------------------------------
    auto path = nav_msgs::msg::Path();
    path.header.stamp    = this->now();
    path.header.frame_id = "base_link";

    bool obstacle_ahead = (nearest_obstacle_x_ < look_ahead_distance_);
    double lateral_offset = obstacle_ahead ? 2.5 : 0.0;  // dodge left by 2.5m

    int steps = 10;
    for (int i = 1; i <= steps; ++i) {
        geometry_msgs::msg::PoseStamped ps;
        ps.header = path.header;
        double s = (look_ahead_distance_ / steps) * i;
        // Simple arc: linear forward + lateral nudge decaying with distance
        double decay = 1.0 - static_cast<double>(i) / steps;
        ps.pose.position.x = s;
        ps.pose.position.y = lateral_offset * decay;
        ps.pose.orientation.w = 1.0;
        path.poses.push_back(ps);
    }

    planned_path_pub_->publish(path);

    if (obstacle_ahead)
        RCLCPP_INFO(this->get_logger(), "Obstacle at %.1fm — planning detour (offset %.1fm)",
            nearest_obstacle_x_, lateral_offset);
    //-- end impl [planning_cycle] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<PathPlanner>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}