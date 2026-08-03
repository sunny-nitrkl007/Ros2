#pragma once
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

namespace lps_core {

template<typename T>
class OrderedQueue {
public:
  explicit OrderedQueue(std::size_t capacity = 20) : capacity_(capacity) {}

  bool push(const T & value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.size() >= capacity_) {
      return false;
    }
    queue_.push_back(value);
    return true;
  }

  std::optional<T> pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return std::nullopt;
    }
    T value = queue_.front();
    queue_.pop_front();
    return value;
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

private:
  const std::size_t capacity_;
  mutable std::mutex mutex_;
  std::deque<T> queue_;
};

}  // namespace lps_core
