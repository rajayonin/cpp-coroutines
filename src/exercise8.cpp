// - Associate an awaiter type with a `task<T>` that
//   - suspends if the coroutine is not suspended at its final suspend point
//   - on resume
//     - does nothing for `task<void>` case
//     - otherwise, returns a result stored in `std::optional`
//   - no action is being done on suspend
//     - we will work on it later

#include <concepts>
#include <coroutine>
#include <memory>
#include <optional>

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


// ********* STORAGE **********

namespace detail {

template<typename T>
class storage {
protected:
  std::optional<T> result;
public:
  using value_type = T;

  template<std::convertible_to<T> U>
  void set_value(U&& value) noexcept(std::is_nothrow_constructible_v<T, decltype(std::forward<U>(value))>)
  {
    result = std::forward<U>(value);
  }
  [[nodiscard]] const T& get() const & { return *result; }
  [[nodiscard]] T&& get() && { return *std::move(result); }
};

template<>
class storage<void> {
public:
  void get() const {}
};

}

// ********* TASK *********

namespace detail {

template<typename T>
struct task_promise_storage_base : storage<T> {
  [[noreturn]] void unhandled_exception() { throw; }
};

template<typename T>
struct task_promise_storage : task_promise_storage_base<T> {
  template<std::convertible_to<T> U>
  void return_value(U&& value) noexcept(noexcept(this->set_value(std::forward<U>(value))))
    requires requires { this->set_value(std::forward<U>(value)); }
  {
    this->set_value(std::forward<U>(value));
  }
};

template<>
struct task_promise_storage<void> : task_promise_storage_base<void> {
  void return_void() noexcept {}
};

} // namespace detail

template<typename T>
concept task_value_type = std::move_constructible<T> || std::is_void_v<T>;

template<task_value_type T>
struct [[nodiscard]] task {
  struct promise_type : detail::task_promise_storage<T> {
    static std::suspend_never initial_suspend() noexcept { return {}; }
    static std::suspend_always final_suspend() noexcept { return {}; }
    task get_return_object() noexcept { return this; }
  };

  [[nodiscard]] decltype(auto) get_result() const & noexcept
  {
    return promise_->get();
  }
  [[nodiscard]] decltype(auto) get_result() const && noexcept
  {
    return std::move(promise_)->get();
  }

  auto operator co_await() {
    struct awaiter {
        promise_type *p_;
        bool await_ready() { return true; }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
            return handle;
        }
        decltype(auto) await_resume () {
            return p_->get();
        }
    };
    // Thanks Gonzalo for this bit :))))))
    return awaiter{promise_.get()};
  }

private:
  task(promise_type* p) : promise_(p) {}
  promise_ptr<promise_type> promise_;
};


// ********* EXAMPLE *********

#include <iostream>

task<int> foo()
{
  co_return 42;
}

task<int> bar()
{
  const int res = co_await foo();
  std::cout << "Result of foo: " << res << "\n";
  co_return res + 23;
}

task<void> baz()
{
  const auto res = co_await bar();
  std::cout << "Result of bar: " << res << "\n";
}

task<void> run()
{
  co_await baz();
}

int main()
{
  const auto task = run();
}
