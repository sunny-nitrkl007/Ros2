#include "safety_node.hpp"

using namespace std::chrono_literals;

SafetyNode::SafetyNode()
: Node("safety_node")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("emergency_brake_threshold", 0.8);
    emergency_brake_threshold_ = this->get_parameter("emergency_brake_threshold").as_double();
    this->declare_parameter<int64_t>("reaction_time_ms", 100);
    reaction_time_ms_ = this->get_parameter("reaction_time_ms").as_int();
    this->declare_parameter<bool>("enable_emergency_override", true);
    enable_emergency_override_ = this->get_parameter("enable_emergency_override").as_bool();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        pub_emergency_brake_pub_ = this->create_publisher<adas_interfaces::msg::EmergencyBrake>("/safety/emergency_brake", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /safety/emergency_brake");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        sub_obstacle_data_sub_ = this->create_subscription<adas_interfaces::msg::ObstacleData>(
            "/perception/obstacle_data", qos,
            std::bind(&SafetyNode::on_sub_obstacle_data, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /perception/obstacle_data");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        sub_trajectory_sub_ = this->create_subscription<adas_interfaces::msg::Trajectory>(
            "/planning/trajectory", qos,
            std::bind(&SafetyNode::on_sub_trajectory, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /planning/trajectory");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    server_check_risk_srv_ = this->create_service<adas_interfaces::srv::CheckRisk>(
        "/safety/check_risk",
        std::bind(&SafetyNode::on_server_check_risk, this,
            std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(this->get_logger(), "Service ready: /safety/check_risk");

}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyNode::on_sub_obstacle_data(const adas_interfaces::msg::ObstacleData::SharedPtr msg)
{
    //-- begin impl [on_sub_obstacle_data] ----------------------------------------
    (void)msg;
    // implement: handle message on /perception/obstacle_data
    //-- end impl [on_sub_obstacle_data] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyNode::on_sub_trajectory(const adas_interfaces::msg::Trajectory::SharedPtr msg)
{
    //-- begin impl [on_sub_trajectory] ----------------------------------------
    (void)msg;
    // implement: handle message on /planning/trajectory
    //-- end impl [on_sub_trajectory] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void SafetyNode::on_server_check_risk(
    const adas_interfaces::srv::CheckRisk::Request::SharedPtr request,
    adas_interfaces::srv::CheckRisk::Response::SharedPtr response)
{
    //-- begin impl [on_server_check_risk] ----------------------------------------
    (void)request;
    (void)response;
    // implement: /safety/check_risk
    //-- end impl [on_server_check_risk] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SafetyNode>());
    rclcpp::shutdown();
    return 0;
}