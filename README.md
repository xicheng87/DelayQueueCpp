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
