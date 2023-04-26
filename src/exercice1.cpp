// - Compiler will guide you :-)
// - Return object of `std::suspend_never` type from suspend points

#include <exception>
#include <future>
#include <iostream>
#include <coroutine>
#include <optional>

template <typename R, typename... Args>
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

std::future<int> foo() {
  co_return 42;
}

int main() {
  std::cout << foo().get() << "\n";
  return 0;
}
