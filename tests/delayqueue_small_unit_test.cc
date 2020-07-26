// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#include <future>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "src/delay_queue.h"
#include "src/threadsafe_queue.h"

class DelayQueueSmallUnitTest : public ::testing::Test {
 protected:
  DelayQueue delay_queue_;
  // Used to store results of int during testing
  ThreadsafeQueue<int> int_res_queue_;

 public:
  int add(int a, int b) {
    int res(a + b);
    int_res_queue_.Push(res);
    return res;
  }
};

// Define several tasks with different delay time and makes sure that they all
// execute with expected returns. In this specific case, we insert tasks 
// in sequence with a delay time gradually increasing
TEST_F(DelayQueueSmallUnitTest, AddTaskInSequence) {
  int num_tasks(10);
  std::vector<std::future<int>> task_future;
  // Insert tasks with different delay
  for (int i = 0; i < num_tasks; i++) {
    task_future.emplace_back(
        delay_queue_.AddTask(i * 200, 
            std::bind(&DelayQueueSmallUnitTest::add, this, i, i + 1)));
  }

  // Wait for tasks to complete 
  for (int i = 0; i < num_tasks; i++) {
    EXPECT_EQ(task_future[i].get(), 2 * i + 1);
  }

  // Check that the results are showing up in sequence in int_res_queue_
  for (int i = 0; i < num_tasks; i++) {
    int val;
    EXPECT_TRUE(int_res_queue_.TryPop(val));
    EXPECT_EQ(val, 2 * i + 1);
  }
  EXPECT_TRUE(int_res_queue_.Empty());
}

// Similar to the above test but with task added with a reverse order, i.e
// the one that has longest delay time is inserted first
TEST_F(DelayQueueSmallUnitTest, AddTaskInReverseSequence) {
  int num_tasks(10);
  std::vector<std::future<int>> task_future;
  // Insert tasks with different delay
  for (int i = 0; i < num_tasks; i++) {
    task_future.emplace_back(
        delay_queue_.AddTask((num_tasks - i) * 200, 
            std::bind(&DelayQueueSmallUnitTest::add, this, i, i + 1)));
  }

  // Wait for tasks to complete 
  for (int i = 0; i < num_tasks; i++) {
    EXPECT_EQ(task_future[i].get(), 2 * i + 1);
  }

  // Check that the results are showing up in reverse sequence in the 
  // int_res_queue_
  for (int i = num_tasks-1; i >= 0; i--) {
    int val;
    EXPECT_TRUE(int_res_queue_.TryPop(val));
    EXPECT_EQ(val, 2 * i + 1);
  }
  EXPECT_TRUE(int_res_queue_.Empty());
}
