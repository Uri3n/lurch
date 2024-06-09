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

        objects/root/generic_root/init_cmd.cpp
        objects/root/generic_root/receive.cpp
        objects/root/generic_root/tasks.cpp
        objects/root/generic_root/root.hpp

        objects/group/group.cpp
        objects/group/group.hpp
        objects/external/chatroom/chatroom.cpp
        objects/external/chatroom/chatroom.hpp

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

        components/instance/instance.cpp
        components/instance/misc.cpp
        components/instance/ssl.cpp
        components/instance/config.cpp
        components/eventlog/eventlog.cpp
        util/macro.hpp
        util/structs.hpp
        util/enums.hpp

        objects/agent/baphomet/receive.cpp
        objects/agent/baphomet/init_cmd.cpp
        objects/agent/baphomet/tasks.cpp
        objects/agent/baphomet/baphomet.hpp


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
    target_include_directories(server PRIVATE vendor/linux) #unfinished
endif()