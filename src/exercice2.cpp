// - Coroutine should suspend at its final suspend point
//   - allows to read its result
//   - requires manual coroutine destruction
// - `task<T>` is constructed with the pointer to the promise type
//   and should store it as a member
// - Store the result in `std::optional<T>`
// - On the event of unhandled exception just rethrow it
//   - good as long as coroutines are synchronous
// - Remember that `task<T>` is a resource wrapper
//   - make it safe, easy to use and hard to abuse

#include <utility>
#include <coroutine>
#include <exception>
#include <future>
#include <iostream>
#include <optional>


/*
Coroutine definition & setup
*/

template<typename T>
// using a task as a wrapper
struct [[nodiscard]] task {
    struct promise_type {

        // we'll save the result value in std::optional
        std::optional<T> result;

        auto initial_suspend() const { return std::suspend_never{}; }
        // suspend in final to allow to read result (need to destroy)
        auto final_suspend() const noexcept { return std::suspend_always{}; }

        auto get_return_object() { return task(this); }
        void return_value(T v) { result = std::move(v); }

        // re-throw exceptions
        [[noreturn]] void unhandled_exception() const { throw; }
    };

    // interface to get the result (using protection)
    [[nodiscard]] const T get_result() const { return *promise_->result; }

    task(const task&) = delete;
    task& operator=(const task&) = delete;
    task(task&& rhs) noexcept : promise_(std::exchange(rhs.promise_, nullptr)) {}
    task& operator=(task&& rhs) noexcept { promise_ = std::exchange(rhs.promise_, nullptr); return *this;}
    ~task() {
      if(!promise_) return;
      auto coro = std::coroutine_handle<promise_type>::from_promise(*promise_);
      if(coro) coro.destroy();
    }

    // protect promise
    private:
        promise_type *promise_;

        // task constructor (need promise)
        task(promise_type* p) : promise_(p) {}
};


task<int> foo() {
  co_return 42;
}


int main() {
  std::cout << foo().get_result() << "\n";
}
