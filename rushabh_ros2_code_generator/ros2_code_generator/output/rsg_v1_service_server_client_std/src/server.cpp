#include "rsg_v1_service_server_client_std/server.hpp"

#include <functional>
#include <sstream>

#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"

using namespace std::chrono_literals;


namespace rsg_v1_service_server_client_std
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
  main_service_ = this->create_service<std_srvs::srv::Trigger>(
    "/robot1/reset",
    std::bind(&ServerNode::on_request, this, std::placeholders::_1, std::placeholders::_2));
}





void ServerNode::on_request(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
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
  response->success = true;
  response->message = "Handled by generated boilerplate placeholder.";
  // USER LOGIC END
}


}  // namespace rsg_v1_service_server_client_std

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rsg_v1_service_server_client_std::ServerNode>();

  rclcpp::executors::SingleThreadedExecutor executor;

  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
