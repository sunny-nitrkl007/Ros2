#include "rclcpp/rclcpp.hpp"
#include "custom_interfaces_pkg/msg/sensor.hpp"
#include "sensor_subscriber.hpp"

Sensor_subscriber::Sensor_subscriber()

    : Node("sensor_subscriber")

{
    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.reliable();
    

    subscription_ = this->create_subscription<custom_interfaces_pkg::msg::Sensor>(
        "sensor_data", qos,
        std::bind(&Sensor_subscriber::callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Sensor subscriber ready on 'sensor_data'.");
}

void Sensor_subscriber::callback(const custom_interfaces_pkg::msg::Sensor::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(),
        "[%s] seq=%d  temp=%.2f C  hum=%.1f %%",
        msg->sensor_id.c_str(), msg->sequence_number,
        msg->temperature, msg->humidity);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Sensor_subscriber>());
    rclcpp::shutdown();
    return 0;
}