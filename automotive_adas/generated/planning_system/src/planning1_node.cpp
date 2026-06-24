#include "planning1_node.hpp"

using namespace std::chrono_literals;

Planning1Node::Planning1Node()
: Node("planning1_node")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("planning_rate_hz", 10.0);
    planning_rate_hz_ = this->get_parameter("planning_rate_hz").as_double();
    this->declare_parameter<double>("max_speed_mps", 30.0);
    max_speed_mps_ = this->get_parameter("max_speed_mps").as_double();
    this->declare_parameter<double>("safe_follow_distance_m", 10.0);
    safe_follow_distance_m_ = this->get_parameter("safe_follow_distance_m").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        pub_trajectory_pub_ = this->create_publisher<adas_interfaces::msg::Trajectory>("/planning/trajectory", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /planning/trajectory");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        sub_obstacle_data_sub_ = this->create_subscription<adas_interfaces::msg::ObstacleData>(
            "/perception/obstacle_data", qos,
            std::bind(&Planning1Node::on_sub_obstacle_data, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /perception/obstacle_data");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        sub_emergency_brake_sub_ = this->create_subscription<adas_interfaces::msg::EmergencyBrake>(
            "/safety/emergency_brake", qos,
            std::bind(&Planning1Node::on_sub_emergency_brake, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /safety/emergency_brake");
    }

}

// ─────────────────────────────────────────────────────────────────────────────
void Planning1Node::on_sub_obstacle_data(const adas_interfaces::msg::ObstacleData::SharedPtr msg)
{
    //-- begin impl [on_sub_obstacle_data] ----------------------------------------
    (void)msg;
    // implement: handle message on /perception/obstacle_data
    //-- end impl [on_sub_obstacle_data] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void Planning1Node::on_sub_emergency_brake(const adas_interfaces::msg::EmergencyBrake::SharedPtr msg)
{
    //-- begin impl [on_sub_emergency_brake] ----------------------------------------
    (void)msg;
    // implement: handle message on /safety/emergency_brake
    //-- end impl [on_sub_emergency_brake] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Planning1Node>());
    rclcpp::shutdown();
    return 0;
}