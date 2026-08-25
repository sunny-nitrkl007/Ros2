#include "ros2_bridge_app/Ros2BridgeApp.hpp"

#include <chrono>
#include <utility>

#include <ais/interface/InterfaceDb.h>

using namespace std::chrono_literals;
using namespace task;

namespace
{
constexpr std::chrono::milliseconds SWITCH_INPUT_PERIOD(100);
constexpr char NODE_NAME[] = "ros2_bridge_app";
constexpr char TOPIC_NAME[] = "switch_input_scs";
}

AbstractTaskCore * task::getTaskImplementation(void)
{
  static Ros2BridgeApp thisTask("ROS2BridgeApp");
  return &thisTask;
}

Ros2BridgeApp::Ros2BridgeApp(const std::string & taskName)
: Task(taskName),
  lpsSaSwitchInput_(nullptr),
  LpsSaSwitchInput(nullptr),
  running_(false)
{
}

Ros2BridgeApp::~Ros2BridgeApp()
{
  cleanup();
}

bool Ros2BridgeApp::initialize()
{
  AIS_LOG_INFO("ROS2BridgeApp::initialize");

  lpsSaSwitchInput_ =
    dynamic_cast<SwitchInputScsInput *>(
      InterfaceDb::fetch("SwitchInputScsInput"));

  if (lpsSaSwitchInput_ == nullptr) {
    AIS_LOG_ERROR("Failed to fetch SwitchInputScsInput from InterfaceDb");
    return false;
  }

  nativeAdapter_ =
    std::make_unique<NativeSwitchInputRosAdapter>(lpsSaSwitchInput_);
  LpsSaSwitchInput = nativeAdapter_.get();

  if (!rclcpp::ok()) {
    int argc = 0;
    char ** argv = nullptr;
    rclcpp::init(argc, argv);
  }

  rosNode_ = std::make_shared<rclcpp::Node>(NODE_NAME);
  publisher_ =
    rosNode_->create_publisher<ros2_bridge_app::msg::SwitchInputScs>(
      TOPIC_NAME, rclcpp::QoS(10));

  running_.store(true);
  bridgeThread_ = std::thread(&Ros2BridgeApp::bridgeThreadFunction, this);
  return true;
}

bool Ros2BridgeApp::executive()
{
  return true;
}

void Ros2BridgeApp::bridgeThreadFunction()
{
  AIS_LOG_INFO("ROS2 bridge worker thread is running");

  while (running_.load() && rclcpp::ok()) {
    ros2_bridge_app::msg::SwitchInputScs STG_Input;

    LpsSaSwitchInput->get(STG_Input);
    publisher_->publish(STG_Input);

    rclcpp::spin_some(rosNode_);
    std::this_thread::sleep_for(SWITCH_INPUT_PERIOD);
  }
}

void Ros2BridgeApp::cleanup()
{
  if (!running_.exchange(false)) {
    return;
  }

  if (bridgeThread_.joinable()) {
    bridgeThread_.join();
  }

  publisher_.reset();
  rosNode_.reset();
  nativeAdapter_.reset();
  LpsSaSwitchInput = nullptr;
  lpsSaSwitchInput_ = nullptr;

  if (rclcpp::ok()) {
    rclcpp::shutdown();
  }

  AIS_LOG_INFO("ROS2BridgeApp::cleanup");
}
