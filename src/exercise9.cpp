// - Implement `generator<T>`
//   - `next()` should resume the coroutine and return `false` if coroutine is suspended at its final
//     suspend point

#include <coroutine>
#include <exception>
#include <memory>


// ********* RAII *********

struct coro_deleter {
  template<typename Promise>
  void operator()(Promise* promise) const noexcept
  {
    auto handle = std::coroutine_handle<Promise>::from_promise(*promise);
    if(handle)
      handle.destroy();
  }
};
template<typename T>
using promise_ptr = std::unique_ptr<T, coro_deleter>;


// ********* GENERATOR *********

template<typename T>
struct [[nodiscard]] generator {

  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  // ...
  struct promise_type {
    T v;

    generator<T> get_return_object() {
        return {handle_type::from_promise(*this)};
    }
    auto await_transform(auto) = delete;
    void unhandled_exception() { throw; }
    void return_void() noexcept {}

    std::suspend_always initial_suspend() noexcept { return {}; };
    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(auto expr) {
        v = expr;
        return {};
    }
  };

  generator(handle_type h): handle_{h} {}
 
  bool next() {
    handle_.resume();
    return handle_ ? !handle_.done() : false;
  }

  T value() {
    return handle_.promise().v;
  }
private:
  handle_type handle_;
};


// ********* EXAMPLE *********

#include <iostream>

generator<int> simple()
{
  // co_await std::suspend_never{}; // should not compile
  co_yield 1;
  co_yield 2;
}

int main()
{
  auto g = simple();
  while (g.next())
    std::cout << g.value() << ' ';
  std::cout << '\n';
}