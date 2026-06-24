#include "perception1_node.hpp"

using namespace std::chrono_literals;

Perception1Node::Perception1Node()
: Node("perception1_node")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("sensor_update_rate_hz", 20.0);
    sensor_update_rate_hz_ = this->get_parameter("sensor_update_rate_hz").as_double();
    this->declare_parameter<double>("obstacle_detection_range_m", 100.0);
    obstacle_detection_range_m_ = this->get_parameter("obstacle_detection_range_m").as_double();
    this->declare_parameter<bool>("enable_object_classification", true);
    enable_object_classification_ = this->get_parameter("enable_object_classification").as_bool();
    this->declare_parameter<bool>("enable_debug_logging", false);
    enable_debug_logging_ = this->get_parameter("enable_debug_logging").as_bool();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        pub_obstacle_data_pub_ = this->create_publisher<adas_interfaces::msg::ObstacleData>("/perception/obstacle_data", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /perception/obstacle_data");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        sub_emergency_state_sub_ = this->create_subscription<adas_interfaces::msg::EmergencyBrake>(
            "/safety/emergency_brake", qos,
            std::bind(&Perception1Node::on_sub_emergency_state, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /safety/emergency_brake");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    server_get_lane_info_srv_ = this->create_service<adas_interfaces::srv::GetLaneInfo>(
        "/perception/get_lane_info",
        std::bind(&Perception1Node::on_server_get_lane_info, this,
            std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(this->get_logger(), "Service ready: /perception/get_lane_info");

}

// ─────────────────────────────────────────────────────────────────────────────
void Perception1Node::on_sub_emergency_state(const adas_interfaces::msg::EmergencyBrake::SharedPtr msg)
{
    //-- begin impl [on_sub_emergency_state] ----------------------------------------
    (void)msg;
    // implement: handle message on /safety/emergency_brake
    //-- end impl [on_sub_emergency_state] ----------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void Perception1Node::on_server_get_lane_info(
    const adas_interfaces::srv::GetLaneInfo::Request::SharedPtr request,
    adas_interfaces::srv::GetLaneInfo::Response::SharedPtr response)
{
    //-- begin impl [on_server_get_lane_info] ----------------------------------------
    (void)request;
    (void)response;
    // implement: /perception/get_lane_info
    //-- end impl [on_server_get_lane_info] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Perception1Node>());
    rclcpp::shutdown();
    return 0;
}