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

#include <coroutine>
#include <exception>
#include <future>
#include <iostream>
#include <optional>

template<typename R, typename... Args>
struct std::coroutine_traits<std::future<R>, Args...> {
  struct promise_type {
    std::promise<R> p;
    auto get_return_object() { return p.get_future(); }
    auto initial_suspend() { return std::suspend_never{}; }
    auto final_suspend() noexcept { return std::suspend_never{}; }
    void return_value(R v) { p.set_value(v); }
    void unhandled_exception() { p.set_exception(std::current_exception()); }
  };
};

std::future<int> foo1()
{
  co_return 42;
}

template<typename T>
struct [[nodiscard]] task {
    struct promise_type {
        std::optional<T> result;
        auto get_return_object() { return task(this); }
        auto initial_suspend() const { return std::suspend_never{}; }
        auto final_suspend() const noexcept { return std::suspend_always{}; }
        void return_value(T v) { result = std::move(v); }
        [[noreturn]] void unhandled_exception() const { throw; }
    };
    [[nodiscard]] const T get_result() const { return *promise_->result; }
private:
    promise_type *promise_;
    task(promise_type* p) : promise_(p) {}
};

task<int> foo2()
{
  co_return 42;
}

int main()
{
  std::cout << foo1().get() << "\n";
  std::cout << foo2().get_result() << "\n";
}
