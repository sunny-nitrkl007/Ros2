#pragma once

#include "rclcpp/rclcpp.hpp"
#include "adas_interfaces/srv/check_risk.hpp"
#include "adas_interfaces/srv/get_lane_info.hpp"
#include "adas_interfaces/srv/request_path_update.hpp"

class Planning2Node : public rclcpp::Node
{
public:
    Planning2Node();

private:
    // Service servers
    rclcpp::Service<adas_interfaces::srv::RequestPathUpdate>::SharedPtr server_request_path_update_srv_;
    void on_server_request_path_update(
        const adas_interfaces::srv::RequestPathUpdate::Request::SharedPtr request,
        adas_interfaces::srv::RequestPathUpdate::Response::SharedPtr response);

    // Service clients
    rclcpp::Client<adas_interfaces::srv::GetLaneInfo>::SharedPtr client_get_lane_info_client_;
    rclcpp::Client<adas_interfaces::srv::CheckRisk>::SharedPtr client_check_risk_client_;

    // Parameters
    double planning_rate_hz_;
    double max_speed_mps_;
    double safe_follow_distance_m_;
};