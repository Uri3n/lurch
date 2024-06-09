//
// Created by diago on 2024-05-24.
//

#ifndef COMMON_HPP
#define COMMON_HPP
#include <string>
#include <vector>
#include <winternl.h>
#include <macro.hpp>

constexpr uint32_t hash_ansi(const char* str);
constexpr size_t comptime_strlen_ansi(const char* string);

template<typename T>
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

#endif //COMMON_HPP
