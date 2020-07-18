// Copyright (c) 2020 Xi Cheng. All rights reserved.
// Use of this source code is governed by a Apache License 2.0 that can be
// found in the LICENSE file.
//
// Credit: this file has been inspired by the following article:
// https://www.crazygaze.com/blog/2016/03/24/portable-c-timer-queue/

#include "src/semaphore.h"

void Semaphore::Notify() {
  std::unique_lock<std::mutex> lock(mutex_);
  count_++;
  condition_variable_.notify_one();
}

void Semaphore::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  condition_variable_.wait(lock, [this]() { return count_ > 0; });
  count_--;
}
