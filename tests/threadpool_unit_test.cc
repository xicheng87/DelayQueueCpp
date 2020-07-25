// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <vector>

#include "gtest/gtest.h"
#include "src/threadpool.h"

// Simple addition function 
int test_add(int a, int b) {
  return a + b;
}

class ThreadPoolUnitTest : public ::testing::Test {
 protected:
  ThreadPool threadpool_;
 
  // Helper function to run multiple addition task with a specified number
  // of repetitions
  void run_multiple_add(int num_tasks) {
    std::vector<std::future<int>> add_futures;
    for (int i = 0; i < num_tasks; i++) {
      // For the i-th task, submit a task to add i and i + 1
      add_futures.push_back(threadpool_.Submit(std::bind(test_add, i, i + 1)));
    }

    for (int i = 0; i < num_tasks; i++) {
      EXPECT_EQ(add_futures[i].get(), 2 * i + 1);
    }
  }
};

// Basic test that a single task can be accomplished via the threadpool
TEST_F(ThreadPoolUnitTest, SingleTask) {
  std::future<int> add_future = threadpool_.Submit(std::bind(test_add, 1, 1));
  int res = add_future.get();
  EXPECT_EQ(res, 2);
}

// Test that multiple tasks can be run with the threadpool
TEST_F(ThreadPoolUnitTest, MultipleTasks) {
  run_multiple_add(100);
}

// Test that a large number of tasks can be added to the threadpool and one
// can wait for the results
TEST_F(ThreadPoolUnitTest, LargeNumberTasks) {
  run_multiple_add(10000);
}
