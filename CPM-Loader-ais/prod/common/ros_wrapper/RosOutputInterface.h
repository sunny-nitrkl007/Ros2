///////////////////////////////////////////////////////////////////////////////
/// @file RosOutputInterface.h
/// @brief DDS-backed drop-in replacement for
///        interfaces::baseTypes::OutputInterface<T>. Matches its publish()
///        call signature exactly so call sites in JobMgr/WeighApp don't
///        change -- only the member's declared type does.
///
/// Header-only, same reason the real SCS OutputInterface<T> is header-only:
/// it's a template, instantiated once per real channel message type.
///
/// Design decisions (Development-Plan.txt, "RESOLVED DECISIONS"):
///   - No PIMPL. <rclcpp/rclcpp.hpp> is visible wherever this header is
///     included. PIMPL doesn't transfer cleanly to a header-only template
///     (no single .cpp to hide Impl in) -- kept as an explicit fallback,
///     only built if this plain approach causes a real problem.
///   - One shared rclcpp::Node per app (passed in), not one node per
///     channel -- each RosOutputInterface<T> just creates its own
///     publisher on that shared node.
///////////////////////////////////////////////////////////////////////////////

#ifndef _RosOutputInterface_h_
#define _RosOutputInterface_h_

#include <string>

#include <rclcpp/rclcpp.hpp>

namespace ros_wrapper
{

template<typename T>
class RosOutputInterface
{
public:
    RosOutputInterface(const rclcpp::Node::SharedPtr& node, const std::string& topicName, size_t qosDepth = 10)
        : topicName_(topicName)
    {
        publisher_ = node->create_publisher<T>(topicName, qosDepth);
    }

    // Matches interfaces::baseTypes::OutputInterface<T>::publish(const T&) --
    // same call syntax as the real SCS object it replaces.
    bool publish(const T& data)
    {
        try
        {
            publisher_->publish(data);
            return true;
        }
        catch (const std::exception&)
        {
            // Mirrors the real OutputInterface<T>::publish() contract: callers
            // (e.g. LpsSaJobMgrScs.cpp:930) check the bool return value rather
            // than expecting an exception to propagate.
            return false;
        }
    }

    const std::string& topicName() const { return topicName_; }

private:
    typename rclcpp::Publisher<T>::SharedPtr publisher_;
    std::string topicName_;
};

} // namespace ros_wrapper

#endif // _RosOutputInterface_h_
