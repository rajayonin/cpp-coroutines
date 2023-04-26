#include <iostream>
#include <coroutine>
#include <exception>
#include <future>

// task<float> foo(std::string x, bool flag);
// using promise = std::coroutine_traints<task<float>, std::string, bool>::promise_type;

template<typename T>
struct task {
  struct promise_type {
    // One of the below
    promise_type() = default;
    // promise_type(coro_args ...);
  
    auto get_return_object();
    void unhandled_exception();
    auto initial_suspend();
    auto final_suspend() noexcept;
  
    // Only one of the below
    void return_value(auto expr);
    // void return_void();


    // Optional
    /*
     * auto yield_value(auto expr);
     * auto get_return_object_on_allocation_failure();
     * auto await_trasnform(auto expr);
     * void *operator new(std::size_t size, args...);
     * void operator delete(void *ptr, std::size_t size);
     */
  };
};

std::future<int> foo() {
  std::promise<int> p;
  auto f = p.get_future();
  try {
    int i = 42;
    p.set_value(i);
  } catch (...) {
    p.set_exception(std::current_exception());
  }
  return f;
}

int main (int argc, char *argv[]) {

  std::cout << "Hello, Corroutines!" << std::endl;
  int x = foo().get();
  std::cout << x << std::endl;
  return 0;
}
