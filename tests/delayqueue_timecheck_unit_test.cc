// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#include <chrono>
#include <ctime>
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

  // Test mode
  enum TaskInsertSequence {
    sequential_,
    reverse_,
    random_
  };

  // Helper function to test different scenario for the single thread case. 
  // This function creates a {num_tasks} number of tasks with an 
  // {interval_milliseconds} duration apart from each other, and would insert 
  // the tasks to delay queue according to the pattern specified by {mode}
  void MultipleTaskTestSingleThread(unsigned int num_tasks, 
      unsigned int interval_milliseconds, TaskInsertSequence mode) {
    std::vector<Task> tasks;
  
    // Get the task objects initialized
    for (unsigned int i = 0; i < num_tasks; i++) {
      tasks.emplace_back(i * interval_milliseconds);
    }

    // Emit tasks
    std::vector<std::future<void>> task_futures(num_tasks);
    switch (mode) {
      case sequential_:
        for (unsigned int i = 0; i < num_tasks; i++) {
          task_futures[i] = delay_queue_.AddTask(tasks[i].delay_milliseconds_,
                                std::bind(&Task::run, &tasks[i]));
        }
        break;
      case reverse_:
        for (unsigned int i = 0; i < num_tasks; i++) {
          task_futures[i] = delay_queue_.AddTask(
                                tasks[num_tasks - i - 1].delay_milliseconds_,
                                std::bind(&Task::run, 
                                    &tasks[num_tasks - i - 1]));
        }
        break;
      case random_:
        // Use the current time as seed for random generator
        std::srand(std::time(nullptr));
        std::vector<int> task_idx;
        for (unsigned int i = 0; i < num_tasks; i++) {
          task_idx.push_back(i);
        }
        // Randomly shuffle the task to be added to the delay queue
        for (unsigned int i = 0; i < num_tasks; i++) {
          int idx = std::rand() % task_idx.size();
          task_futures[i] = delay_queue_.AddTask(
                                tasks[task_idx[idx]].delay_milliseconds_,
                                std::bind(&Task::run, &tasks[task_idx[idx]]));
          // Remove the task index selected
          task_idx.erase(task_idx.begin() + idx);
        }
        break;
    }

    // Wait for task to complete
    for (auto& task_future : task_futures) {
      task_future.get();
    }
  }
};

// Single task, ensure that the timing is correct
TEST_F(DelayQueueTimeCheckUnitTest, SingleTask) {
  Task task(1000); // Delay time 1s
  auto task_future = delay_queue_.AddTask(task.delay_milliseconds_,
                                          std::bind(&Task::run, &task));
  // Wait for the task to complete
  task_future.get();
}

// Test multiple tasks added in sequence
TEST_F(DelayQueueTimeCheckUnitTest, MultipleTaskSequential) {
  MultipleTaskTestSingleThread(10, 1000, TaskInsertSequence::sequential_);
}

// Test multiple tasks added in reverse sequence
TEST_F(DelayQueueTimeCheckUnitTest, MultipleTaskReverse) {
  MultipleTaskTestSingleThread(10, 1000, TaskInsertSequence::reverse_);
}

// Test multiple tasks added in random order
TEST_F(DelayQueueTimeCheckUnitTest, MultipleTaskRandom) {
  MultipleTaskTestSingleThread(10, 1000, TaskInsertSequence::random_);
}
