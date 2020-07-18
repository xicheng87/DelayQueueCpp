// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <mutex>
#include <condition_variable>

// Definition of a Semaphore class that one can use for notifying, wait and
// and wait for a period of time. Because C++ does not have a built-in semaphore
// until C++20, it is necessary to define one if it is convenient to use it. 
class Semaphore {
 public:
  // Default count is zero, which means calling Wait() immediately after
  // constructor this thread will go to sleep (waiting for count > 0)
  Semaphore(unsigned int count = 0) : count_(count) {}
 
  // Increment the counter and notify one of the thread that there is one 
  // available
  void Notify();

  // Wait for the counter to become positive, and consume one by decrementing
  void Wait();

  // Wait for the counter to become positive until a specified time point at
  // deadline. If the counter becomes positive then consume it and return true,
  // otherwise return false
  template <class Clock, class Duration>
  bool WaitUntil(const std::chrono::time_point<Clock, Duration>& deadline) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!condition_variable_.wait_until(lock, deadline, [this]() { 
        return count_ > 0; })) {
       return false;
    }
   
    count_--;
    return true;
  }
 
private:
    std::mutex mutex_;
    std::condition_variable condition_variable_;
    unsigned int count_;
};

#endif // SEMAPHORE_H_
