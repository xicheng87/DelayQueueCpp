// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#include <chrono>
#include <vector>

#include "gtest/gtest.h"
#include "src/delay_queue.h"

struct Task {
  using TimePoint = std::chrono::high_resolution_clock::time_point;
 
  // Constructor, setting the scheduled time
  Task(uint64_t delay_milliseconds) : 
     scheduledTime_(std::chrono::high_resolution_clock::now()), 
     delay_milliseconds_(delay_milliseconds) {}
 
  // A trivial task run function to ensure the timing is correct
  void run() {
    executeTime_ = std::chrono::high_resolution_clock::now();
    auto delay_duration_milliseconds = std::chrono::duration_cast<
        std::chrono::milliseconds>(executeTime_ - scheduledTime_).count();
    // Make sure that the actual delay duration is higher than or equal to 
    // the configured value
    EXPECT_GE(delay_duration_milliseconds, delay_milliseconds_);
    // Make sure that the difference between the actual delay duration 
    // and the configured value is within an acceptable range
    EXPECT_LE(delay_duration_milliseconds - delay_milliseconds_, 
              epsilon_milliseconds_);
  }

  // Two timepoint indicating the time when a task is scheduled and the time
  // that it gets executed
  TimePoint scheduledTime_;
  TimePoint executeTime_;
  // The delay expressed in milliseconds
  uint64_t delay_milliseconds_;
  // A small amount of time duration when checking the executed time. Note
  // that here this value represents 0.1 second, and the task's delay time
  // in this test is on the scale of seconds. We expect this level of 
  // tolerance to work fine in this non-stress test
  uint64_t epsilon_milliseconds_ = 100; 
};

class DelayQueueTimeCheckUnitTest : public ::testing::Test {
 protected:
  DelayQueue delay_queue_; 
};

// Single task, ensure that the timing is correct
TEST_F(DelayQueueTimeCheckUnitTest, SingleTask) {
  Task task(1000); // Delay time 1s
  auto task_future = delay_queue_.AddTask(task.delay_milliseconds_,
                                          std::bind(&Task::run, &task));
  // Wait for the task to complete
  task_future.get();
}

// Multiple task, ensure that all timings are correct
TEST_F(DelayQueueTimeCheckUnitTest, MultipleTask) {
  std::vector<Task> tasks;
  int num_tasks(10);
  
  // Get the task objects initialized
  for (int i = 0; i < num_tasks; i++) {
    tasks.emplace_back(i * 1000);
  }
 
  // Emit tasks
  std::vector<std::future<void>> task_futures(num_tasks);
  for (int i = 0; i < num_tasks; i++) {
    task_futures[i] = delay_queue_.AddTask(tasks[i].delay_milliseconds_,
                                           std::bind(&Task::run, &tasks[i]));
  }

  // Wait for task to complete
  for (auto& task_future : task_futures) {
    task_future.get();
  }
}
