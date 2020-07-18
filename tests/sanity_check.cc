#include "gtest/gtest.h"
#include "src/delay_queue.h"

class SanityCheck : public ::testing::Test {
};

void foo(int first_arg, int second_arg) {
}

// An empty test case, making sure that what is defined in delay_queue.h can compile
TEST_F(SanityCheck, Empty) {
}

// Define a task with specified delay time and function
TEST_F(SanityCheck, DefineTask) {
  DelayQueue::Task task(0, foo, 100, 1000);
}
