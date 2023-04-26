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


/*
Destructor
*/

struct coroutine_deleter {
    template<typename Promise>
    void operator() (Promise *promise) const noexcept {
        auto handle = std::coroutine_handle<Promise>::from_promise(*promise);
        if (handle)
        handle.destroy();
    }
};

template<typename T>
using promise_ptr = std::unique_ptr<T, coroutine_deleter>;


// say you can move it
template<std::move_constructible T>
struct [[nodiscard]] task {
  struct promise_type {

        std::optional<T> result;

        // static, noexcept + change return types to make better interface
        static std::suspend_never initial_suspend() noexcept { return {}; }
        static std::suspend_always final_suspend() noexcept { return {}; }

        task get_return_object() noexcept { return this; }
        void return_value(T v) noexcept { result = std::move(v); }

        [[noreturn]] static void unhandled_exception() { throw; }
     };

    [[nodiscard]] const T get_result() const noexcept { return *promise_->result; }

    private:
        
        promise_ptr<promise_type> promise_;

        // constructor
        task(promise_type* p) : promise_(p) {}
};


task<int> foo() {
    co_return 42;
}


int main() {
    std::cout << foo().get_result() << "\n";
}
