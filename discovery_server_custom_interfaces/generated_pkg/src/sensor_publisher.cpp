#include "rclcpp/rclcpp.hpp"
#include "custom_interfaces_pkg/msg/sensor.hpp"
#include "sensor_publisher.hpp"
#include <cmath>

using namespace std::chrono_literals;

Sensor_publisher::Sensor_publisher()

    : Node("sensor_publisher")

{
    this->declare_parameter<double>("publish_rate", 1.0);
    this->declare_parameter<std::string>("sensor_id", "temp_sensor_01");

    publish_rate_ = this->get_parameter("publish_rate").as_double();
    sensor_id_    = this->get_parameter("sensor_id").as_string();

    rclcpp::QoS qos(rclcpp::KeepLast(10));

    
    qos.reliable();
    

    publisher_ = this->create_publisher<custom_interfaces_pkg::msg::Sensor>("sensor_data", qos);

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
        std::bind(&Sensor_publisher::publish_message, this));

    RCLCPP_INFO(this->get_logger(),
        "Sensor publisher ready: id='%s'  rate=%.1f Hz  topic='sensor_data'",
        sensor_id_.c_str(), publish_rate_);
}

void Sensor_publisher::publish_message()
{
    auto msg = custom_interfaces_pkg::msg::Sensor();
    msg.sensor_id       = sensor_id_;
    msg.temperature     = 22.0 + std::sin(static_cast<double>(count_) * 0.2) * 5.0;
    msg.humidity        = 55.0 + std::cos(static_cast<double>(count_) * 0.15) * 10.0;
    msg.sequence_number = static_cast<int32_t>(count_++);

    RCLCPP_INFO(this->get_logger(),
        "[%s] seq=%d  temp=%.2f C  hum=%.1f %%",
        msg.sensor_id.c_str(), msg.sequence_number,
        msg.temperature, msg.humidity);
    publisher_->publish(msg);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Sensor_publisher>());
    rclcpp::shutdown();
    return 0;
}