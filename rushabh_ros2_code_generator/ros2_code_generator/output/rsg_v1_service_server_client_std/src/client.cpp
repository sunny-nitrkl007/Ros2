#include "rsg_v1_service_server_client_std/client.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_service_server_client_std
{

ClientNode::ClientNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("client", options)
{
  setup_clients();
  RCLCPP_INFO(this->get_logger(), "Node 'client' started.");
}

ClientNode::~ClientNode()
{
  RCLCPP_INFO(this->get_logger(), "Node 'client' shutting down.");
}






void ClientNode::setup_clients()
{
  main_client_ = this->create_client<std_srvs::srv::Trigger>(
    "/robot1/reset");
}





bool ClientNode::wait_for_main_client()
{
  return main_client_->wait_for_service(std::chrono::milliseconds(1000));
}

void ClientNode::call_main_client_async()
{
  if (!wait_for_main_client()) {
    RCLCPP_WARN(this->get_logger(), "Service /robot1/reset not available.");
    return;
  }
  auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
  // USER LOGIC PLACEHOLDER: fill request for client 'main_client' here.
  auto future = main_client_->async_send_request(
    request,
    [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture future_response) {
      // USER LOGIC PLACEHOLDER: handle response for client 'main_client' here.
      (void)future_response;
    });
  (void)future;
}

}  // namespace rsg_v1_service_server_client_std

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_service_server_client_std::ClientNode>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
