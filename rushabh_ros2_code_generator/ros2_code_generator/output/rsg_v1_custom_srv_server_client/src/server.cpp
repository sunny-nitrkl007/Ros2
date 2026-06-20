#include "rsg_v1_custom_srv_server_client/server.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_custom_srv_server_client
{

ServerNode::ServerNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("server", options)
{
  setup_services();
  RCLCPP_INFO(this->get_logger(), "Node 'server' started.");
}

ServerNode::~ServerNode()
{
  RCLCPP_INFO(this->get_logger(), "Node 'server' shutting down.");
}





void ServerNode::setup_services()
{
  main_service_ = this->create_service<rsg_v1_custom_srv_server_client::srv::SetMode>(
    "/robot1/set_mode",
    std::bind(&ServerNode::on_request, this, std::placeholders::_1, std::placeholders::_2));
}





void ServerNode::on_request(
  const std::shared_ptr<rsg_v1_custom_srv_server_client::srv::SetMode::Request> request,
  std::shared_ptr<rsg_v1_custom_srv_server_client::srv::SetMode::Response> response)
{
  // ==========================================================================
  // USER LOGIC START: fill service response for 'main_service'
  // --------------------------------------------------------------------------
  // Purpose:
  //   Inspect the request and fill the response. If this service has a
  //   control block in grammar, generated logic updates the target node
  //   parameter/state from the request field.
  // ==========================================================================
  (void)request;
  (void)response;
  // USER LOGIC END
}


}  // namespace rsg_v1_custom_srv_server_client

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_custom_srv_server_client::ServerNode>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
