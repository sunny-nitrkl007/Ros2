#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_interfaces/action/fibonacci.hpp"
#include "fibonacci_client.hpp"

Fibonacci_client::Fibonacci_client()

    : Node("fibonacci_client")

{
    this->declare_parameter<int>("default_order",    10);
    this->declare_parameter<double>("goal_interval_s", 5.0);

    default_order_   = this->get_parameter("default_order").as_int();
    goal_interval_s_ = this->get_parameter("goal_interval_s").as_double();

    action_client_ = rclcpp_action::create_client<example_interfaces::action::Fibonacci>(this, "/fibonacci");

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(goal_interval_s_ * 1000.0)),
        std::bind(&Fibonacci_client::send_goal, this));

    RCLCPP_INFO(this->get_logger(),
        "Action client ready. Sending goals to '%s': order=%d every %.1f s.",
        "/fibonacci", default_order_, goal_interval_s_);
}

void Fibonacci_client::send_goal()
{
    if (!action_client_->wait_for_action_server(std::chrono::seconds(1))) {
        RCLCPP_WARN(this->get_logger(), "Action server not available, waiting...");
        return;
    }

    auto goal  = example_interfaces::action::Fibonacci::Goal();
    goal.order = default_order_;
    RCLCPP_INFO(this->get_logger(), "Sending goal: order=%d", goal.order);

    auto options = rclcpp_action::Client<example_interfaces::action::Fibonacci>::SendGoalOptions();

    options.goal_response_callback = [this](
        const rclcpp_action::ClientGoalHandle<example_interfaces::action::Fibonacci>::SharedPtr & handle) {
            if (!handle) {
                RCLCPP_ERROR(this->get_logger(), "Goal rejected by server.");
            } else {
                RCLCPP_INFO(this->get_logger(), "Goal accepted by server.");
            }
        };

    options.feedback_callback = [this](
        rclcpp_action::ClientGoalHandle<example_interfaces::action::Fibonacci>::SharedPtr,
        const std::shared_ptr<const example_interfaces::action::Fibonacci::Feedback> feedback) {
            const auto & seq = feedback->partial_sequence;
            RCLCPP_INFO(this->get_logger(),
                "Feedback: %zu values so far, latest F=%d",
                seq.size(), seq.empty() ? -1 : seq.back());
        };

    options.result_callback = [this](
        const rclcpp_action::ClientGoalHandle<example_interfaces::action::Fibonacci>::WrappedResult & result) {
            switch (result.code) {
                case rclcpp_action::ResultCode::SUCCEEDED: {
                    const auto & seq = result.result->sequence;
                    RCLCPP_INFO(this->get_logger(),
                        "Result: %zu Fibonacci numbers computed, last=%d",
                        seq.size(), seq.empty() ? -1 : seq.back());
                    break;
                }
                case rclcpp_action::ResultCode::CANCELED:
                    RCLCPP_WARN(this->get_logger(), "Goal was canceled.");
                    break;
                default:
                    RCLCPP_ERROR(this->get_logger(), "Goal failed.");
                    break;
            }
        };

    action_client_->async_send_goal(goal, options);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Fibonacci_client>());
    rclcpp::shutdown();
    return 0;
}