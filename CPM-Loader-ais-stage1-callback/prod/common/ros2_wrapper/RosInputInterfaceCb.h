///////////////////////////////////////////////////////////////////////////////
/// @file RosInputInterfaceCb.h
/// @brief PROTOTYPE, not wired into any app. Callback-capable sibling of
///        RosInputInterface.h -- answers "can we get addNewDataSlot()-style
///        callbacks back?" (yes) and shows what it actually costs.
///
///        RosInputInterface<T>'s subscription lambda already IS a callback
///        -- ROS2 invokes it the instant a message arrives -- it just isn't
///        exposed. This class exposes it via setNewDataCallback(), same
///        idea as the real SCS InputInterface<T>::addNewDataSlot().
///
///        The real cost: RosInputInterface<T> gets away with no mutex
///        because its subscription callback only ever fires during
///        executor_.spin_some(), which the app calls synchronously from
///        its own executive() tick -- same thread as get(), never
///        concurrent. A *notification* callback is only useful if it can
///        fire independently of that tick (otherwise it's just the same
///        drain*() call moved a few lines earlier -- see chat discussion).
///        That means the executor has to spin on its own thread (see
///        LpsSaJobMgrRosChannelsCb.h), which means the subscription
///        callback below now runs on a DIFFERENT thread than get()/
///        setNewDataCallback() -- hence the mutex.
///////////////////////////////////////////////////////////////////////////////

#ifndef _RosInputInterfaceCb_h_
#define _RosInputInterfaceCb_h_

#include <queue>
#include <string>
#include <mutex>
#include <functional>

#include <rclcpp/rclcpp.hpp>

namespace ros2_wrapper
{

template<typename T>
class RosInputInterfaceCb
{
public:
    // void() -- matches the real SCS InputInterface<T>::addNewDataSlot()
    // signature exactly. It's a pure "something new arrived" notification,
    // not a data-carrying callback: the handler is expected to call get()
    // itself to retrieve what's actually pending, same as the original
    // notifyResponseInput()/notifyTxInput() do.
    using Callback = std::function<void()>;

    RosInputInterfaceCb(const rclcpp::Node::SharedPtr& node, const std::string& topicName, size_t qosDepth = 10)
        : topicName_(topicName)
    {
        subscription_ = node->create_subscription<T>(
            topicName,
            qosDepth,
            [this](const typename T::SharedPtr msg)
            {
                // Runs on whatever thread is spinning the executor -- the
                // background thread in LpsSaJobMgrRosChannelsCb.h, NOT
                // necessarily the thread calling get() below.
                Callback cbCopy;
                {
                    std::lock_guard<std::mutex> lck(mtx_);
                    queue_.push(*msg);
                    cbCopy = callback_; // copy while locked, invoke unlocked
                }
                // Invoked outside the lock: DDSWeighAppInfCb's own
                // notifyResponseInput()/notifyTxInput() (registered here as
                // the callback) call get() on this same object, and get()
                // below also takes mtx_ -- calling the callback while still
                // holding the lock would deadlock.
                if (cbCopy) {
                    cbCopy();
                }
            });
    }

    // Same drain semantics as RosInputInterface<T>::get() -- now thread-safe.
    bool get(T& data)
    {
        std::lock_guard<std::mutex> lck(mtx_);
        if (queue_.empty())
        {
            return false;
        }
        data = queue_.front();
        queue_.pop();
        return true;
    }

    // addNewDataSlot()-equivalent. Real SCS used boost::signals2 to support
    // multiple listeners per channel; every real caller in this codebase
    // (LpsSaWeighAppInf, DDSWeighAppInf) only ever registers one, so a
    // single std::function is a faithful-enough stand-in for this prototype.
    void setNewDataCallback(Callback cb)
    {
        std::lock_guard<std::mutex> lck(mtx_);
        callback_ = cb;
    }

    const std::string& topicName() const { return topicName_; }

private:
    typename rclcpp::Subscription<T>::SharedPtr subscription_;
    std::mutex mtx_;
    std::queue<T> queue_;
    Callback callback_;
    std::string topicName_;
};

} // namespace ros2_wrapper

#endif // _RosInputInterfaceCb_h_
