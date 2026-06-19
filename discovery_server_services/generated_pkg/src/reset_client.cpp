#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "reset_client.hpp"

using namespace std::chrono_literals;

Reset_client::Reset_client()

    : Node("reset_client")

{
    client_ = this->create_client<std_srvs::srv::Trigger>("/reset_system");
    timer_  = this->create_wall_timer(
        2000ms, std::bind(&Reset_client::send_request, this));
    RCLCPP_INFO(this->get_logger(), "Client ready, calling '%s' every 2 s.", "/reset_system");
}

void Reset_client::send_request()
{
    if (!client_->wait_for_service(1s)) {
        RCLCPP_WARN(this->get_logger(), "Service not available, waiting...");
        return;
    }
    auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
    auto future  = client_->async_send_request(
        request,
        [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture f) {
            auto resp = f.get();
            RCLCPP_INFO(this->get_logger(), "Response: success=%s  message='%s'",
                resp->success ? "true" : "false", resp->message.c_str());
        });
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Reset_client>());
    rclcpp::shutdown();
    return 0;
}