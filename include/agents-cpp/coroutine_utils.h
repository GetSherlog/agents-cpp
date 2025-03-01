#pragma once

#include <agents-cpp/types.h>
#include <vector>
#include <memory>

// Try to include from both possible locations
#if __has_include(<folly/experimental/coro/Task.h>)
  #include <folly/experimental/coro/Task.h>
  #include <folly/experimental/coro/AsyncGenerator.h>
  #include <folly/experimental/coro/BlockingWait.h>
  #include <folly/experimental/coro/AsyncScope.h>
  #include <folly/io/async/ScopedEventBaseThread.h>
  #include <folly/executors/CPUThreadPoolExecutor.h>
  #include <folly/executors/GlobalExecutor.h>
  #define HAS_FOLLY_CORO 1
#elif __has_include(<folly/coro/Task.h>)
  #include <folly/coro/Task.h>
  #include <folly/coro/AsyncGenerator.h>
  #include <folly/coro/BlockingWait.h>
  #include <folly/coro/AsyncScope.h>
  #include <folly/io/async/ScopedEventBaseThread.h>
  #include <folly/executors/CPUThreadPoolExecutor.h>
  #include <folly/executors/GlobalExecutor.h>
  #define HAS_FOLLY_CORO 1
#else
  #include <functional>
  #include <future>
  #define HAS_FOLLY_CORO 0
#endif

namespace agents {

#if HAS_FOLLY_CORO
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
#else
// Provide a future-based fallback for Task
template <typename T>
class Task {
private:
    std::function<std::future<T>()> _func;
public:
    Task(std::function<std::future<T>()> func) : _func(std::move(func)) {}
    std::future<T> get_future() { return _func(); }
};

// A minimal AsyncGenerator implementation that doesn't rely on coroutines
template <typename T>
class AsyncGenerator {
private:
    std::function<std::vector<T>()> _func;
public:
    AsyncGenerator(std::function<std::vector<T>()> func) : _func(std::move(func)) {}
    std::vector<T> collect() { return _func(); }
};

// Helper to run a task and get the result synchronously
template <typename T>
T blockingWait(Task<T>&& task) {
    auto future = task.get_future();
    return future.get();
}

// Helper to collect all results from an AsyncGenerator
template <typename T>
std::vector<T> collectAll(AsyncGenerator<T>&& generator) {
    return generator.collect();
}

// A minimal executor implementation
class Executor {
public:
    template <typename F>
    void add(F&& f) {
        std::thread(std::forward<F>(f)).detach();
    }
};

// Get a global executor
inline Executor* getExecutor() {
    static Executor executor;
    return &executor;
}
#endif

} // namespace agents 