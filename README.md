[![Build Status](https://travis-ci.com/xicheng87/DelayQueueCpp.svg?branch=master&status=passed)](https://travis-ci.com/xicheng87/DelayQueueCpp)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# DelayQueueCpp
A delay-queue implemented in C++ that defers execution of tasks by a period of time
that is configurable by users. 

# Features

* Allows users to specify a delay period for a given task
* Returns a `std::future` for the task specified so users can wait for the completion
  of task and retrieve the return value
* Provides high throughput of processing via thread-pools designed underneath

# Building

This project is built via [Bazel](http://bazel.build), which can be downloaded and
installed at [here](https://docs.bazel.build/versions/master/install.html).

## MacOs

You can use Homebrew:
```
brew tap bazelbuild/tap
brew install bazelbuild/tap/bazel
```

## Linux

You can execute the `scripts/install.sh` after specifying appropriate Bazel version
in the shell. 
```
export BAZEL_VERSION=3.1.0
./scripts/install.sh
```

## Build and Test

To build and test your branch, simply execute the following
```
bazel build ...
bazel test ...
```

To build in development mode with debug symbols, run the following:
```
bazel build -c dbg ...
bazel test -c dbg ...
```

You can go to [Bazel's Official Documentations](https://docs.bazel.build/versions/master/bazel-overview.html)
to learn more about how to use it.

# How to use

You should specify a task by wrapping the function and parameters via the `std::bind`
call. For example, the following code snippet defines a delay queue and add a task to
execute function `foo` with parameters 1 and 2:

```
#include "delay_queue.h"

int foo(int a, int b) {
  return a + b;
}

DelayQueue delay_queue;

std::future<int> task_future = delay_queue.AddTask(
                   /* Delay period in milliseconds  */ 1000,
                   std::bind(&foo, 1, 2));

int res = task_future.get();
```
where the first parameter taken by DelayQueue::AddTask is an unsigned integer
representing the delay period in milliseconds, and the second parameter is a callable
object returned by `std::bind`. The DelayQueue gaurantees you that it won't start
execute the specified task after the configured amount of time.  

You can also specify class member functions as tasks. For example, for a class member
function `Foo::task`, you use `std::bind(&Foo::task, ...)` to create the collable
object that a delay queue takes.

# Caveat

TBD

# References

TBD
