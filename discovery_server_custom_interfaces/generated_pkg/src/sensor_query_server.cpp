#include "rclcpp/rclcpp.hpp"
#include "custom_interfaces_pkg/srv/sensor_query.hpp"
#include "sensor_query_server.hpp"

Sensor_query_server::Sensor_query_server()

    : Node("sensor_query_server")

{
    service_ = this->create_service<custom_interfaces_pkg::srv::SensorQuery>(
        "/sensor_query",
        std::bind(&Sensor_query_server::handle_request, this,
            std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(this->get_logger(), "Sensor query service '/sensor_query' ready.");
}

void Sensor_query_server::handle_request(
    const custom_interfaces_pkg::srv::SensorQuery::Request::SharedPtr request,
    custom_interfaces_pkg::srv::SensorQuery::Response::SharedPtr response)
{
    RCLCPP_INFO(this->get_logger(),
        "Query received: sensor_id='%s'", request->sensor_id.c_str());

    if (request->sensor_id == "temp_sensor_01") {
        response->sensor_found = true;
        response->temperature  = 23.5;
        response->humidity     = 58.2;
        response->message      = "OK";
    } else if (request->sensor_id == "temp_sensor_02") {
        response->sensor_found = true;
        response->temperature  = 19.1;
        response->humidity     = 71.0;
        response->message      = "OK";
    } else {
        response->sensor_found = false;
        response->temperature  = 0.0;
        response->humidity     = 0.0;
        response->message      = "Sensor not found: " + request->sensor_id;
    }

    RCLCPP_INFO(this->get_logger(),
        "Response: found=%s  temp=%.2f C  hum=%.1f %%  msg='%s'",
        response->sensor_found ? "true" : "false",
        response->temperature, response->humidity,
        response->message.c_str());
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Sensor_query_server>());
    rclcpp::shutdown();
    return 0;
}