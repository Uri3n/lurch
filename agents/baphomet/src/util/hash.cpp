//
// Created by diago on 2024-06-06.
//

#include <common.hpp>

constexpr size_t
comptime_strlen_ansi(const char* string) {

    size_t counter = 0;
    while (*string != '\0') {

        ++counter;
        ++string;
    }

    return counter;
}


constexpr uint32_t
hash_ansi(const char* str) {

    uint32_t initial_seed = 7; //can be altered if needed
    uint32_t hash         = 0;
    size_t index          = 0;
    size_t len            = comptime_strlen_ansi(str);

    while(index != len) {
        hash += str[index++];
        hash += hash << initial_seed;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

