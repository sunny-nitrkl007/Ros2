///////////////////////////////////////////////////////////////////////////////
/// @file RosInputInterface.h
/// @brief DDS-backed drop-in replacement for
///        interfaces::baseTypes::InputInterface<T>. Matches its get() call
///        signature and drain semantics exactly, so call sites like
///        `while (channel->get(x)) { ... }` throughout LpsSaJobMgrScs.cpp
///        keep working unchanged.
///
///    - Backed by a std::queue<T>, not a single cached latest-value.
///     Real SCS get() is called in drain loops everywhere in the existing
///     codebase -- if two messages arrive between polls, both must be
///     retrievable, not just the newest.
///////////////////////////////////////////////////////////////////////////////
 
#ifndef _RosInputInterface_h_
#define _RosInputInterface_h_
 
#include <queue>
#include <string>
 
#include "rclcpp/rclcpp.hpp"
 
namespace ros2_wrapper
{
 
template<typename T>
class RosInputInterface
{
public:
    RosInputInterface(const rclcpp::Node::SharedPtr& node, const std::string& topicName, size_t qosDepth = 10)
        : topicName_(topicName)
    {
        subscription_ = node->create_subscription<T>(
            topicName,
            qosDepth,
            [this](const typename T::SharedPtr msg)
            {
                queue_.push(*msg);
            });
    }
 
    // Matches interfaces::baseTypes::InputInterface<T>::get(T&) -- returns
    // true and fills `data` with the oldest still-unread message if one is
    // pending, false if the queue is empty. Call in a while loop to drain
    // everything received since the last poll.
    bool get(T& data)
    {
        if (queue_.empty())
        {
            return false;
        }
        data = queue_.front();
        queue_.pop();
        return true;
    }
 
    const std::string& topicName() const { return topicName_; }
 
private:
    typename rclcpp::Subscription<T>::SharedPtr subscription_;
    std::queue<T> queue_;
    std::string topicName_;
};
 
} // namespace ros2_wrapper
 
#endif // _RosInputInterface_h_