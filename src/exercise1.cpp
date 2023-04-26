// - Compiler will guide you :-)
// - Return object of `std::suspend_never` type from suspend points

#include <exception>
#include <future>
#include <iostream>
#include <coroutine>
#include <optional>



std::future<int> old_foo() {
    /*
    Original way to do this.
    */
    std::promise<int> p;
    auto f = p.get_future();
    try {
      int i = 42;
      p.set_value(i);
    }
    catch(...) {
      p.set_exception(std::current_exception());
    }
    return f;
}


/*
Coroutine definition & setup
*/

template <typename R, typename... Args>
struct std::coroutine_traits<std::future<R>, Args...> {
    /*
    Coroutine interface
    */
   
    // define promise type
    struct promise_type {

        // declare promise
        std::promise<R> p;
        
        // suspension options - never suspend
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }

        // return options
        void return_value(R v) { p.set_value(v); }
        auto get_return_object() { return p.get_future(); }

        // exceptions
        void unhandled_exception() { p.set_exception(std::current_exception()); }
    };
};


/*
Coroutine function
*/
std::future<int> foo() {
  co_return 42;
}



int main() {
  std::cout << foo().get() << "\n";
  return 0;
}
