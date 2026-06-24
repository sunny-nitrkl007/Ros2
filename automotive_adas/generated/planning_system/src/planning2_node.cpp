#include "planning2_node.hpp"

using namespace std::chrono_literals;

Planning2Node::Planning2Node()
: Node("planning2_node")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("planning_rate_hz", 10.0);
    planning_rate_hz_ = this->get_parameter("planning_rate_hz").as_double();
    this->declare_parameter<double>("max_speed_mps", 30.0);
    max_speed_mps_ = this->get_parameter("max_speed_mps").as_double();
    this->declare_parameter<double>("safe_follow_distance_m", 10.0);
    safe_follow_distance_m_ = this->get_parameter("safe_follow_distance_m").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Service servers ───────────────────────────────────────────────────────
    server_request_path_update_srv_ = this->create_service<adas_interfaces::srv::RequestPathUpdate>(
        "/planning/request_path_update",
        std::bind(&Planning2Node::on_server_request_path_update, this,
            std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(this->get_logger(), "Service ready: /planning/request_path_update");

    // ── Service clients ───────────────────────────────────────────────────────
    client_get_lane_info_client_ = this->create_client<adas_interfaces::srv::GetLaneInfo>("/perception/get_lane_info");
    RCLCPP_INFO(this->get_logger(), "Client created: /perception/get_lane_info");
    client_check_risk_client_ = this->create_client<adas_interfaces::srv::CheckRisk>("/safety/check_risk");
    RCLCPP_INFO(this->get_logger(), "Client created: /safety/check_risk");

}

// ─────────────────────────────────────────────────────────────────────────────
void Planning2Node::on_server_request_path_update(
    const adas_interfaces::srv::RequestPathUpdate::Request::SharedPtr request,
    adas_interfaces::srv::RequestPathUpdate::Response::SharedPtr response)
{
    //-- begin impl [on_server_request_path_update] ----------------------------------------
    (void)request;
    (void)response;
    // implement: /planning/request_path_update
    //-- end impl [on_server_request_path_update] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Planning2Node>());
    rclcpp::shutdown();
    return 0;
}