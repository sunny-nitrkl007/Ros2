#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_interfaces/action/fibonacci.hpp"
#include "fibonacci_server.hpp"

#include <thread>

using namespace std::chrono_literals;

Fibonacci_server::Fibonacci_server()

    : Node("fibonacci_server")

{
    this->declare_parameter<int>("step_delay_ms", 500);
    step_delay_ms_ = this->get_parameter("step_delay_ms").as_int();

    action_server_ = rclcpp_action::create_server<example_interfaces::action::Fibonacci>(
        this,
        "/fibonacci",
        std::bind(&Fibonacci_server::handle_goal,     this,
            std::placeholders::_1, std::placeholders::_2),
        std::bind(&Fibonacci_server::handle_cancel,   this,
            std::placeholders::_1),
        std::bind(&Fibonacci_server::handle_accepted, this,
            std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(),
        "Action server '%s' ready. step_delay_ms=%d",
        "/fibonacci", step_delay_ms_);
}

rclcpp_action::GoalResponse Fibonacci_server::handle_goal(
    const rclcpp_action::GoalUUID & /*uuid*/,
    std::shared_ptr<const example_interfaces::action::Fibonacci::Goal> goal)
{
    RCLCPP_INFO(this->get_logger(), "Received goal request: order=%d", goal->order);
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse Fibonacci_server::handle_cancel(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<example_interfaces::action::Fibonacci>> /*goal_handle*/)
{
    RCLCPP_INFO(this->get_logger(), "Cancel request received.");
    return rclcpp_action::CancelResponse::ACCEPT;
}

void Fibonacci_server::handle_accepted(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<example_interfaces::action::Fibonacci>> goal_handle)
{
    std::thread([this, goal_handle]() { execute(goal_handle); }).detach();
}

void Fibonacci_server::execute(
    const std::shared_ptr<rclcpp_action::ServerGoalHandle<example_interfaces::action::Fibonacci>> goal_handle)
{
    const int order = goal_handle->get_goal()->order;
    RCLCPP_INFO(this->get_logger(),
        "Executing: Fibonacci(%d) with %d ms between steps.", order, step_delay_ms_);

    auto feedback = std::make_shared<example_interfaces::action::Fibonacci::Feedback>();
    auto & seq    = feedback->sequence;
    seq.push_back(0);
    seq.push_back(1);

    for (int i = 2; i < order && rclcpp::ok(); ++i) {
        if (goal_handle->is_canceling()) {
            auto result      = std::make_shared<example_interfaces::action::Fibonacci::Result>();
            result->sequence = seq;
            goal_handle->canceled(result);
            RCLCPP_INFO(this->get_logger(), "Goal cancelled at step %d.", i);
            return;
        }
        seq.push_back(seq[i - 1] + seq[i - 2]);
        goal_handle->publish_feedback(feedback);
        RCLCPP_INFO(this->get_logger(),
            "Feedback [%d/%d]: F(%d)=%d", i, order - 1, i, seq.back());
        std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms_));
    }

    auto result      = std::make_shared<example_interfaces::action::Fibonacci::Result>();
    result->sequence = seq;
    goal_handle->succeed(result);
    RCLCPP_INFO(this->get_logger(),
        "Goal succeeded. Sequence length=%zu, last value=%d",
        seq.size(), seq.back());
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Fibonacci_server>());
    rclcpp::shutdown();
    return 0;
}