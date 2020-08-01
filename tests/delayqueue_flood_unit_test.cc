// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#include <future>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "src/delay_queue.h"

class DelayQueueFloodUnitTest : public ::testing::Test {
 protected:
  DelayQueue delay_queue_;
};

int foo(int a, int b) {
  return a + b;
}

// Have 10 threads and each threads emit 10000 tasks at the same delay time. 
// This will "flood" the delay queue and checks that all tasks finish
TEST_F(DelayQueueFloodUnitTest, ManyTasksSameDelayTime) {
  int num_tasks(10);
  int num_tasks_per_thread(10000);
  uint64_t delay(100);
  std::vector<std::thread> threads;

  for (int i = 0; i < num_tasks; i++) {
    threads.push_back(std::thread( [i, delay, num_tasks_per_thread, this] () {
      std::vector<std::future<int>> task_futures;
      for (int j = 0; j < num_tasks_per_thread; j++) {
        task_futures.push_back(
            this->delay_queue_.AddTask(delay, std::bind(&foo, i, i+1)));
      }

      for (auto& task_future : task_futures) {
        EXPECT_EQ(task_future.get(), 2 * i + 1);
      }
    }));
  }

  for (auto& th : threads) {
    th.join();
  }
  threads.clear();
}
