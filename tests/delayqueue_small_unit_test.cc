// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#include <future>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "src/delay_queue.h"

class DelayQueueSmallUnitTest : public ::testing::Test {
 protected:
  DelayQueue delay_queue_;
};

int add(int a, int b) {
  std::cout << a << " " << b << std::endl; 
  return a + b;
}

// Define several tasks with different delay time and makes sure that they all
// execute with expected returns. In this specific case, we insert tasks 
// in sequence with a delay time gradually increasing
TEST_F(DelayQueueSmallUnitTest, AddTaskInSequence) {
  int num_tasks(10);
  std::vector<std::future<int>> task_future;
  // Insert tasks with different delay
  for (int i = 0; i < num_tasks; i++) {
    task_future.emplace_back(
        delay_queue_.AddTask(i * 200, std::bind(add, i, i + 1)));
  }

  // Wait for tasks to complete 
  for (int i = 0; i < num_tasks; i++) {
    EXPECT_EQ(task_future[i].get(), 2 * i + 1);
  }
}
