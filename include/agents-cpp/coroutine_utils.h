#pragma once

#include <agents-cpp/types.h>
#include <folly/experimental/coro/Task.h>
#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/AsyncScope.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/GlobalExecutor.h>

namespace agents {

// Type aliases for Folly coroutine types
template <typename T>
using Task = folly::coro::Task<T>;

template <typename T>
using AsyncGenerator = folly::coro::AsyncGenerator<T>;

// Helper to run a coroutine and get the result synchronously
template <typename T>
T blockingWait(Task<T>&& task) {
    return folly::coro::blockingWait(std::move(task));
}

// Helper to run an async generator and collect results
template <typename T>
std::vector<T> collectAll(AsyncGenerator<T>&& generator) {
    return blockingWait(folly::coro::collectAll(std::move(generator)));
}

// Executor for running coroutines
inline folly::Executor* getExecutor() {
    static folly::CPUThreadPoolExecutor executor(
        std::thread::hardware_concurrency(),
        std::make_shared<folly::NamedThreadFactory>("AgentExecutor"));
    return &executor;
}

} // namespace agents 