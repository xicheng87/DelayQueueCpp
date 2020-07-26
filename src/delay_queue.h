// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#ifndef DELAY_QUEUE_H_
#define DELAY_QUEUE_H_

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include "src/semaphore.h"
#include "src/threadpool.h"

class DelayQueue {
  // A Task class that allows one to pass an arbitrary function that
  // takes an arbitrary number of parameters. The constructor takes the function
  // address, followed by a list of arguments that the function will call upon.
  // To execute the task, one submits its function_wrapper to the thread pool
  struct Task {
    // Create a delayed task by specifying the delay duration in milliseconds,
    // followed by the function pointer and a list of arguments
    Task(uint64_t delay_milliseconds, FunctionWrapper&& function_wrapper) : 
        function_wrapper_(std::move(function_wrapper)) {
      auto now = std::chrono::high_resolution_clock::now();
      start_time_ = now + std::chrono::milliseconds(delay_milliseconds);
    }

    bool operator>(const Task& other) const {
      return start_time_ > other.start_time_;
    }

    // Indicate the timepoint for this task to start
    std::chrono::high_resolution_clock::time_point start_time_;
    // Function wrapper for the task's function
    FunctionWrapper function_wrapper_;
  };

 public:
  DelayQueue() : terminated_(false), 
      dispatch_thread_(std::thread([this] () { wait_and_dispatch(); })) {
  }

  ~DelayQueue();

  // Add a task, which is specified by a delay period and a callable object
  // Return a future object so that the caller of this function can wait for
  // the task and fetch results
  template <typename Function>
  std::future<typename std::result_of<Function()>::type> 
      AddTask(uint64_t delay_milliseconds, Function function) {
    // Create a packaged_task and prepare the future object that a user gets
    // to use, and to wait for this task
    typedef typename std::result_of<Function()>::type result_type;
    std::packaged_task<result_type()> task(std::move(function));
    std::future<result_type> res(task.get_future());

    // Lock the task queue and insert a task underneath
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace(delay_milliseconds, std::move(task)); 

    // Notify the dispatch thread as a new task is created
    semaphore_.Notify();
    return res;
  }
  
 private:
  // The dispatching thread runs this function to wait for new tasks to come
  // and to dispatch them when their delay time has elapsed
  void wait_and_dispatch();

  // Helper function to compute the next time period to wait before waking up
  // again. If the first field is false, it means there is currently no task
  // in the queue, otherwise, the second field represents the time duration
  // between now and the task's start time on top of the queue
  std::pair<bool, std::chrono::high_resolution_clock::time_point>
      compute_next_wait_until_time();

  // Helper function to dispatch the tasks on top of the queue to the 
  // threadpool as much as possible, as long as the tasks' start_time is 
  // before now
  void dispatch();

  // Just an alias of computing now timepoint
  std::chrono::high_resolution_clock::time_point now() const {
    return std::chrono::high_resolution_clock::now();
  }

  // A mutex to protect the whole delay queue
  std::mutex mutex_;
  // A semaphore used by the delay queue to synchronize task insertion
  Semaphore semaphore_;

  // A priority queue that is used for delay queue, and the top task in the
  // priority queue is the task that is assigned for the nearest future, i.e.
  // the minimal start_time
  std::priority_queue<Task, std::vector<Task>, std::greater<Task>> task_queue_;

  // A flag to indiate whether the delay queue has been terminated
  std::atomic<bool> terminated_;

  // The thread that reacts to the addition of tasks and is responsible for 
  // popping the next task at the right time and dispatch to working 
  // thread pool
  std::thread dispatch_thread_;

  // The threadpool that executes the tasks
  ThreadPool worker_thread_pool_;
};

#endif // DELAY_QUEUE_H_
