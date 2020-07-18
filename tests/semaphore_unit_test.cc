#include <atomic>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "src/semaphore.h"

class SemaphoreUnitTest : public ::testing::Test {
};

// Initialize a semaphore with count 1, and verify that wait won't block
TEST_F(SemaphoreUnitTest, InitCountAsOneAndSingleWait) {
  Semaphore semaphore(1);
  semaphore.Wait();
}

// Initialize a sempahore with count 0, and wait until a timeout
TEST_F(SemaphoreUnitTest, InitCountAsZeroAndSingleWaitUntil) {
  Semaphore sempahore;
  // Set the timeout duration to be 1 second
  auto now = std::chrono::high_resolution_clock::now();
  auto wait_duration = std::chrono::milliseconds(1000);
  
  // Because there is only one thread and because the count is initialized as 
  // zero, we expect WaitUntil to timeout and therefore return false
  EXPECT_FALSE(sempahore.WaitUntil(now + wait_duration));
  
  // Just some sanity check on the time that elapses
  auto now2 = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(now2 - now >= wait_duration);
}

// A rather simple multi-threaded case that one thread runs as producer which
// notifies and the other runs as a consumer which wait on the semaphore
// Repeat this exercise for 100 times
TEST_F(SemaphoreUnitTest, InitCountAsOneAndOneWaitAndOneNotify) {
  Semaphore semaphore;
  for (int i = 0; i < 100; i++) {
    auto producer_thread(std::thread([&semaphore] () { semaphore.Notify(); }));
    auto consumer_thread(std::thread([&semaphore] () { semaphore.Wait(); }));
    // We expect both threads to complete so join should return successfully
    producer_thread.join();
    consumer_thread.join();
  }
}

// Initialize a semaphore with a count of 10, and have 20 consumer threads to
// wait on this semaphore. 10 of them returns immediately and the remaining
// 10 timeout
TEST_F(SemaphoreUnitTest, InitCountAsTenWithTwentyConsumers) {
  Semaphore semaphore(10);
  std::atomic<int> timeouts{0};
  auto const wait_duration = std::chrono::milliseconds(1000);
  std::vector<std::thread> threads;

  for (int i = 0; i < 20; i++) {
    threads.push_back(std::thread([&semaphore, &wait_duration, &timeouts] () {
      auto now = std::chrono::high_resolution_clock::now();
      auto wait_result = semaphore.WaitUntil(now + wait_duration);
      // Increment the timeouts counter if a consumer thread times out on wait
      if (!wait_result) {
        timeouts++;
      }
    }));
  }

  // Join the threads and clear the placeholder for threads
  for (auto& t : threads) {
    t.join();
  }
  threads.clear();

  EXPECT_EQ(timeouts.load(), 10);
}
