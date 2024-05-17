cmake_minimum_required(VERSION 3.28)
project(server)

set(CMAKE_CXX_STANDARD 23)
set(WINDOWS_VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/windows)
set(LINUX_VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/linux)

add_executable(server main.cpp
        vendor/sqlite/sqlite3.c
        vendor/sqlite/sqlite_modern_cpp.h

        objects/base/base.cpp
        objects/base/base.hpp

        util/argument_parser.cpp
        util/argument_parser.hpp
        util/common.hpp
        util/io.cpp
        util/io.hpp

        objects/agent/baphomet.cpp
        objects/agent/baphomet.hpp
        objects/root/root.cpp
        objects/root/root.hpp
        objects/group/group.cpp
        objects/group/group.hpp

        components/instance.cpp
        components/instance.hpp

        components/database/queries.cpp
        components/database/restore_objects.cpp
        components/database/fileman.cpp
        components/database/misc.cpp

        components/routing/endpoints.cpp
        components/routing/misc.cpp
        components/routing/websocket.cpp
        components/routing/handlers.cpp

        components/tree/send.cpp
        components/tree/misc.cpp
)

target_compile_definitions(server PRIVATE CROW_ENABLE_SSL)

if(WIN32)
    target_include_directories(server PRIVATE ${WINDOWS_VENDOR_DIR})
    target_link_directories(server PRIVATE ${WINDOWS_VENDOR_DIR})
    target_link_libraries(server PRIVATE libcrypto)
    target_link_libraries(server PRIVATE libssl)
    target_link_libraries(server PRIVATE crypt32)
    target_link_libraries(server PRIVATE ws2_32)
endif()

if(UNIX)
    target_include_directories(server PRIVATE vendor/linux)
endif()