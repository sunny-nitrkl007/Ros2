#pragma once
#include <chrono>
#include <mutex>
#include <optional>

namespace lps_core {

template<typename T>
class LatestValueCache {
public:
  using Clock = std::chrono::steady_clock;

  void update(const T & value) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ = value;
    received_at_ = Clock::now();
  }

  std::optional<T> snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return value_;
  }

  bool has_value() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return value_.has_value();
  }

  bool is_fresh(std::chrono::milliseconds maximum_age) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return value_.has_value() && (Clock::now() - received_at_) <= maximum_age;
  }

private:
  mutable std::mutex mutex_;
  std::optional<T> value_;
  Clock::time_point received_at_{Clock::time_point::min()};
};

}  // namespace lps_core
