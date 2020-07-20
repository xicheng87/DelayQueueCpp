// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.

#include <atomic>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "src/threadsafe_queue.h"

class ThreadSafeQueueUnitTest : public ::testing::Test {
};

// A generalized function to run a number of producer and consumer threads
// where a producer enqueues and a consumer dequeues. If the number of
// producers >= the number of consumers, one can simply call WaitAndPop
// in the consumers. Otherwise, call TryPop in the consumers so that they
// do not block. Record the returned false and verify that 
void producuer_consumer_test(int num_producer, int num_consumer) {
  ThreadsafeQueue<int> t_queue;
  std::vector<std::thread> threads; 
  bool const less_producer(num_producer < num_consumer);
  std::atomic<int> num_try_pop_fail{0};

  for (int i = 0; i < num_producer; i++) {
    threads.push_back(std::thread([i, &t_queue] () {
      t_queue.Push(i);
    }));
  }

  for (int i = 0; i < num_consumer; i++) {
    threads.push_back(std::thread([i, less_producer, 
                                   &t_queue, &num_try_pop_fail] () {
      int value;
      if (less_producer) {
        num_try_pop_fail += (!t_queue.TryPop(value));
      } else {
        t_queue.WaitAndPop(value);   
      }
    }));
  }

  for (auto& t : threads) {
    t.join();
  }
  threads.clear();

  EXPECT_EQ(num_try_pop_fail.load(), 
            less_producer ? num_consumer - num_producer : 0);
}

// Initialize a ThreadsafeQueue, use it in a single-thread situation 
TEST_F(ThreadSafeQueueUnitTest, BasicCase) {
  ThreadsafeQueue<int> t_queue;
  int num_of_items(100);

  EXPECT_TRUE(t_queue.Empty());
  for (int i = 0; i < num_of_items; i++) {
    t_queue.Push(i);
  }

  for (int i = 0; i < num_of_items; i++) {
    int val;
    EXPECT_TRUE(t_queue.TryPop(val));
    EXPECT_EQ(val, i);
  }
}

// Test the case that the number of producer is equal to the number of consumer
TEST_F(ThreadSafeQueueUnitTest, EqualProducerAndConsumer) {
  producuer_consumer_test(100, 100);  
}

// More producer
TEST_F(ThreadSafeQueueUnitTest, MoreProducerThanConsumer) {
  producuer_consumer_test(100, 20);
}

// Less producer
TEST_F(ThreadSafeQueueUnitTest, LessProducerThanConsumer) {
  producuer_consumer_test(100, 200);
}
