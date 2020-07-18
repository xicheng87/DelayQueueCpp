// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#ifndef DELAY_QUEUE_H_
#define DELAY_QUEUE_H_

#include <chrono>
#include <functional>
#include <memory>

#include "src/semaphore.h"

namespace DelayQueue {

// A generalized Task class that allows one to pass an arbitrary function that
// takes an arbitrary number of parameters. The constructor takes the function
// address, followed by a list of arguments that the function will call upon.
// To execute the task, one calls Execute().
class Task {
 public:
  template <typename Function, typename... Args>
  // Create a delayed task by specifying the delay duration in milliseconds,
  // followed by the function pointer and a list of arguments
  Task(uint64_t delayInMilliseconds, Function function, Args&&... args) {
    function_internal_ = std::bind(function, std::forward<Args>(args)...);
    auto now = std::chrono::high_resolution_clock::now();
    deadline_ = now + std::chrono::milliseconds(delayInMilliseconds);
  }

  // Access the deadline time_point
  const std::chrono::high_resolution_clock::time_point deadline() const {
    return deadline_;
  }

  // Execute the task
  void Execute() {
    function_internal_();
  }

 private:
  std::function<void()> function_internal_;
  std::chrono::high_resolution_clock::time_point deadline_;
};

} // namespace DelayQueue

#endif // DELAY_QUEUE_H_
