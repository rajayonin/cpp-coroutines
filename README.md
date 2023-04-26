# cpp-coroutines

## Exercise 1: Our Future?
Make the below coroutine compile and work as expected:
```cpp
std::future<int> foo(){
    co_return 42;
}
```

Return object of `std::suspend_never` type from suspend points.  

[Link to statement](https://godbolt.org/z/x3f8fWs5n).


## Exercise 2: A "simple" task

- Coroutine should suspend at its final suspend point
    - allows to read its result
    - requires manual coroutine destruction
- `task<T>` is constructed with the pointer to the promise type and should store it as a member
- Store the result in `std::optional<T>`
- On the event of unhandled exception just rethrow it
    - food as long as coroutines are synchronous
- Remember that `task<T>` is a resource wrapper 
    - make it safe, easy to use and hard to abuse

```cpp
template <typename T>
struct [[nodiscard]] task {
    // ...
};
```

```cpp
task<int> foo() {
    co_return 42;
}
```

```cpp
std::cout << foo().get_result() << std::endl
```


[Link to statement](https://godbolt.org/z/qEW5dT3sz).


## Exercise 3: Better task

Make exercise 2 better.


## Exercise 4: Task of `void`

Extend our task to support no return type as well.

```cpp
task <int> foo() {
    co_return 42;
}

task<void> coro() {
    std::cout << foo().get_result() << std::endl
    co_return;
}
```

```c
auto c = coro();
```

[Link to statement](https://godbolt.org/z/aKvbKeecj).


## Exercise 5: `sleep_for()`

Implement the `sleep_for()` awaiter type that will suspend a current thread for a specified duration.

```cpp
template<specialization_of<std::chrono::duration> D>
struct sleep_for {
    D duration;
};
```

```cpp
task<void> foo() {
    using namespace std::chrono_literals;

    std::cout << "about to sleep\n";
    co_await sleep_for(1s);
    std::cout << "about to return\n";
}
```

```cpp
auto task = foo();
```

[Link to statement](https://godbolt.org/z/qq4xsqPqn).


## Exercise 6: `std::chrono::duration` as awaitable

Refactor the previous exercise to directly work with `std::chrono::duration`

```cpp
task<void> foo() {
    using namespace std::chrono_literals;

    std::cout << "about to sleep\n";
    co_await 1s;
    std::cout << "about to return\n";
}
```

[Link to statement](https://godbolt.org/z/vrEenj6TT).


## Exercise 7: Making `std::chrono::duration` contextually awaitable

Refactor the previous exercise to use our `std::chrono::duration`-specific awaitable implementation only while awaiting in `task<T>`-based coroutine.

```cpp
using namespace std::chrono_literals;

task<void> foo() {
    co_await std::suspend_never{};

    std::cout << "about to sleep\n";
    co_await 1s;

    std::cout << "about to sleep again\n";
    auto dur = 1s;
    co_await dur;
    std::cout << "about to return\n";
}
```

```cpp
std::future<void> boo()
{
  co_await std::suspend_never{};
  std::cout << "You shall not sleep!\n";
  co_await 1s;  // Should not compile
}
```

```cpp
int main()
{
  auto task = foo();
  auto fut = boo();
}
```

[Link to statement](https://godbolt.org/z/58x6cPe7T).


## Exercise 8: Awaiting tasks

Associate an awaiter type with a `task<T>` that:
- suspends if the coroutine is not suspended at its final suspend point
- on resume:
    - does nothing for `task<void>` case
    - otherwise, returns a result stored in `std::optional`
- no action is being done on suspend

```cpp
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
```

[Link to statement](https://godbolt.org/z/bPPjsv4K5).



# Installation and execution

In order to compile the source code there are two ways:
 * Using CMake
 * Using GPRbuild

## Using CMake
Create a `build` directory, go inside and run `cmake`.

```bash
mkdir -pv build
cd build
cmake ..
make
```

The executables are under `build/src/`.

## Using GPRbuild
Just run `gprbuild`. The executables are under `bin`.
