// Refactor the previous exercise to directly work with `std::chrono::duration`

#include <concepts>
#include <coroutine>
#include <memory>
#include <optional>

struct coro_deleter {
    template<typename Promise>
    void operator()(Promise* promise) const noexcept {
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
        void set_value(U&& value) noexcept(std::is_nothrow_constructible_v<T, decltype(std::forward<U>(value))>) {
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

    [[nodiscard]] decltype(auto) get_result() const & noexcept {
        return promise_->get();
    }

    [[nodiscard]] decltype(auto) get_result() const && noexcept {
        return std::move(promise_)->get();
    }

    private:
        task(promise_type* p) : promise_(p) {}
        promise_ptr<promise_type> promise_;
};


// ********* EXAMPLE *********

#include <chrono>
#include <iostream>
#include <thread>

template<typename T, template<typename...> typename Type>
inline constexpr bool is_specialization_of = false;

template<typename... Params, template<typename...> typename Type>
inline constexpr bool is_specialization_of<Type<Params...>, Type> = true;

template<typename T, template<typename...> typename Type>
concept specialization_of = is_specialization_of<T, Type>;

// sleep_for implementation
template<specialization_of<std::chrono::duration> D>
struct sleep_for {
    D duration;

    // called at beggining of coroutine, returns if value is ready (false)
    bool await_ready() const noexcept { return duration <= D::zero(); }

    // returns true when value is ready
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coro) const {
        std::this_thread::sleep_for(duration);
        return coro;
    }

        // executed when finished
        static void await_resume() noexcept {}
};


task<void> foo() {
    using namespace std::chrono_literals;

    std::cout << "about to sleep\n";
    co_await sleep_for(1s);
    std::cout << "about to return\n";
}


int main() {
    auto task = foo();
}
