#include "perception2_node.hpp"

using namespace std::chrono_literals;

Perception2Node::Perception2Node()
: Node("perception2_node")
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

    // ── Service clients ───────────────────────────────────────────────────────
    client_request_path_update_client_ = this->create_client<adas_interfaces::srv::RequestPathUpdate>("/planning/request_path_update");
    RCLCPP_INFO(this->get_logger(), "Client created: /planning/request_path_update");

}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Perception2Node>());
    rclcpp::shutdown();
    return 0;
}