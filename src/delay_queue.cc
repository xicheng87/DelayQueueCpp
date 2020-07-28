// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.

#include "src/delay_queue.h"

DelayQueue::~DelayQueue() {
  // Turn off the delay queue by setting the terminated flag and join the thread
  terminated_.store(true);
  // We need to wake up the dispatch thread in case it is stuck in the Wait() 
  // due to an empty queue
  semaphore_.Notify();
  dispatch_thread_.join();
}

void
DelayQueue::wait_and_dispatch() {
  while (!terminated_.load()) {
    auto next_time_point(compute_next_wait_until_time());
    // If there is a task in the queue, we call WaitUntil from the semaphore
    // This lets this thread to sleep up to next_wait_time before either 
    // woken up by a new task, or when the top task is ready to run
    if (next_time_point.first) {
      semaphore_.WaitUntil(next_time_point.second);
    } else {
      semaphore_.Wait();
    }
    
    // After waking up, dispatch as many as possible
    dispatch();
  }
}

std::pair<bool, std::chrono::high_resolution_clock::time_point>
DelayQueue::compute_next_wait_until_time() {
  // Lock the mutex and return the start_time
  std::lock_guard<std::mutex> lock(mutex_);
  if (!task_queue_.empty()) {
    return std::make_pair(true, task_queue_.top().start_time_);
  }

  return std::make_pair(false, std::chrono::high_resolution_clock::time_point());
}

void
DelayQueue::dispatch() {
  // Lock the mutex and keep popping the task on top of the task queue until 
  // the start_time is after now
  std::lock_guard<std::mutex> lock(mutex_);
  while (!task_queue_.empty() && task_queue_.top().start_time_ <= now()) {
    // There is no easy way to move the top item out of the priority queue's 
    // top element without performing the following const cast. This is mainly
    // because top() returns a const T&, which cannot bind to T&&. 
    auto task(std::move(const_cast<Task&>(task_queue_.top())));
    task_queue_.pop();
    worker_thread_pool_.Submit(std::move(task.function_wrapper_));
  }
}
