// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>

#include "src/threadsafe_queue.h"

// Definition of a function wrapper that stores the reference to a function.
// This is necessary as a task queue expects a copyable object 
class FunctionWrapper {
 public:
  template <typename Function>
  FunctionWrapper(Function&& function) : 
      impl_(new ImplType<Function>(std::move(function))) {}

  // Basically a functor pattern, use () operator to invoke the call
  void operator() () {
    impl_->Call();
  }

  FunctionWrapper() = default;

  // Move constructor
  FunctionWrapper(FunctionWrapper&& other) : impl_(std::move(other.impl_)) {}

  // Move assignment operator 
  FunctionWrapper& operator= (FunctionWrapper&& other) {
    impl_ = std::move(other.impl_);
    return *this;
  }

  // Remove other copy constructors
  FunctionWrapper(const FunctionWrapper&) = delete;
  FunctionWrapper(FunctionWrapper&) = delete;
  FunctionWrapper& operator= (const FunctionWrapper&) = delete;

 private:
  // A base type for implementation
  struct ImplBase {
    virtual void Call() = 0;
    virtual ~ImplBase() {}
  };
  std::unique_ptr<ImplBase> impl_;

  template <typename Function>
  // Actual impl type, which stores a function reference and provides a 
  // unified interface to invoke this function
  struct ImplType : ImplBase {
    Function func;
    ImplType(Function&& _func) : func(std::move(_func)) {}
    void Call() {
      func();
    }
  };
};

// Definition of a simple thread pool class. This implementation is essentially
// taken from Anthony D. Williams, "C++ Concurrency in Action", Chapter 9, 
// section 9.1.1
class ThreadPool {
 public:
  ThreadPool();
  ~ThreadPool();

  // Submit a function to the workpool
  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> 
      Submit(FunctionType function) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(std::move(function));
    std::future<result_type> res(task.get_future());
    work_queue_.Push(std::move(task));
    return res;
  }

 private:
  // An atomic bool to indicate if the thread pool is still operating
  std::atomic<bool> terminated_;
  // A threadsafe queue to store the functions to be called
  ThreadsafeQueue<FunctionWrapper> work_queue_;
  // All threads
  std::vector<std::thread> threads_;

  // Function that runs a worker thread
  void WorkerThread();
};

#endif // THREADPOOL_H_
