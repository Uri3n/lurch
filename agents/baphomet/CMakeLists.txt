cmake_minimum_required(VERSION 3.28)
project(baphomet)

set(CMAKE_CXX_STANDARD 17)

add_library(baphomet SHARED main.cpp
        main.hpp
        networking/send.cpp
        networking/send.hpp
        util/structs.hpp
        util/function_ptrs.hpp
        util/common.hpp
        networking/init.cpp
        networking/init.hpp
        networking/recieve.cpp
        networking/recieve.hpp
        obfuscation/sleep.cpp
        obfuscation/sleep.hpp
)
