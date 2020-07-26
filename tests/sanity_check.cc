// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
#include <iostream>

#include "gtest/gtest.h"
#include "src/delay_queue.h"

class SanityCheck : public ::testing::Test {
 protected:
  DelayQueue delay_queue_;
};

void foo(int first_arg, int second_arg) {
  std::cout << first_arg << " " << second_arg << std::endl;
}

// An empty test case, making sure that what is defined in delay_queue.h can 
// compile
TEST_F(SanityCheck, Empty) {
}

// Define a task with specified delay time and function
TEST_F(SanityCheck, DefineTask) {
  auto foo_future(delay_queue_.AddTask(0, std::bind(foo, 1, 2)));
  foo_future.get();
}
