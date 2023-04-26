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