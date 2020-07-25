// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.

#ifndef THREADSAFE_QUEUE_H_
#define THREADSAFE_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

// A full class definition for a thread-safe queue using condition variables
// This implementation is essentially taken from Anthony D. Williams,
// "C++ Concurrency in Action", Chapter 6, section 6.2.2
template <typename T>
class ThreadsafeQueue {
 public:
  // Push a new value into the queue
  void Push(T new_value) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(std::move(new_value));
    condition_variable_.notify_one();
  }

  // Wait until there is an item in the queue and pop it from the queue, the 
  // popped item is moved to the value variable
  void WaitAndPop(T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_variable_.wait(lock, [this] { return !queue_.empty(); });
    value = std::move(queue_.front());
    queue_.pop();
  }

  bool TryPop(T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    // Empty queue, nothing to pop
    if (queue_.empty()) {
      return false;
    }

    // Pop the front item, move it to value and return true
    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  bool Empty() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
  }

 private:
  // THe thread-safe queue is managed by a normal queue which is protected by
  // a mutex and condition variable
  // The reason that we make mutex mutable is because Empty() method is a const
  // method, which would expect mutex_ to be const as well. However, mutex
  // needs to be intrinsically mutable so therefore we define it as mutable 
  mutable std::mutex mutex_;
  std::queue<T> queue_;
  std::condition_variable condition_variable_;
};

#endif // THREADSAFE_QUEUE_H_
