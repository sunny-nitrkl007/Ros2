#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "reset_server.hpp"

Reset_server::Reset_server()

    : Node("reset_server")

{
    service_ = this->create_service<std_srvs::srv::Trigger>(
        "/reset_system",
        std::bind(&Reset_server::handle_request, this,
            std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(this->get_logger(), "Service '%s' ready.", "/reset_system");
}

void Reset_server::handle_request(
    const std_srvs::srv::Trigger::Request::SharedPtr /*request*/,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    RCLCPP_INFO(this->get_logger(), "Received service request.");
    response->success = true;
    response->message = "OK from reset_server";
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Reset_server>());
    rclcpp::shutdown();
    return 0;
}