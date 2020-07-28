// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.

#include "src/threadpool.h"

// Initialize the threadpool by starting a number of threads 
ThreadPool::ThreadPool() : terminated_(false) {
  auto thread_counts(std::max(std::thread::hardware_concurrency(), 
                     (unsigned int)1));
  try {
    for (unsigned int i = 0; i < thread_counts; i++) {
      threads_.push_back(std::thread(&ThreadPool::WorkerThread, this));
    }
  } catch (...) {
    terminated_.store(true);
  }
}

ThreadPool::~ThreadPool() {
  // Set the terminated flag and join the worker threads
  terminated_.store(true);
  // Notify all worker threads so that they can wake up from wait and terminate
  for(unsigned int i = 0; i < threads_.size(); i++) {
    semaphore_.Notify();
  }
  // Join all worker threads
  for(unsigned int i = 0; i < threads_.size(); i++) {
    threads_[i].join();
  }
}

void
ThreadPool::WorkerThread() {
  // Keep trying pop the task and execute
  while (!terminated_.load()) {
    // Wait for a submission to wake up and process a task
    semaphore_.Wait();

    FunctionWrapper task;
    if (work_queue_.TryPop(task)) {
      task();
    } else {
      std::this_thread::yield();
    }
  }
}
