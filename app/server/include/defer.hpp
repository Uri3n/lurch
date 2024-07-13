//
// Created by diago on 2024-07-12.
//

#ifndef DEFER_HPP
#define DEFER_HPP
#include <type_traits>

template<typename T> requires std::is_invocable_v<T>
class defer_wrapper {
    T callable;
public:

    auto call() -> decltype(callable()) {
        return callable();
    }

    explicit defer_wrapper(T func) : callable(func) {}
    ~defer_wrapper() { callable(); }
};

template<typename T>
defer_wrapper<T> defer(T callable) {
    return defer_wrapper<T>(callable);
}

#endif //DEFER_HPP
