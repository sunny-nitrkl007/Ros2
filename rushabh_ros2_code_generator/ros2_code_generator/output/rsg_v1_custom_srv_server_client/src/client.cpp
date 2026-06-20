#include "rsg_v1_custom_srv_server_client/client.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_custom_srv_server_client
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
  main_client_ = this->create_client<rsg_v1_custom_srv_server_client::srv::SetMode>(
    "/robot1/set_mode");
}





bool ClientNode::wait_for_main_client()
{
  return main_client_->wait_for_service(std::chrono::milliseconds(1000));
}

void ClientNode::call_main_client_async()
{
  if (!wait_for_main_client()) {
    RCLCPP_WARN(this->get_logger(), "Service /robot1/set_mode not available.");
    return;
  }
  auto request = std::make_shared<rsg_v1_custom_srv_server_client::srv::SetMode::Request>();
  // USER LOGIC PLACEHOLDER: fill request for client 'main_client' here.
  auto future = main_client_->async_send_request(
    request,
    [this](rclcpp::Client<rsg_v1_custom_srv_server_client::srv::SetMode>::SharedFuture future_response) {
      // USER LOGIC PLACEHOLDER: handle response for client 'main_client' here.
      (void)future_response;
    });
  (void)future;
}

}  // namespace rsg_v1_custom_srv_server_client

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_custom_srv_server_client::ClientNode>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
