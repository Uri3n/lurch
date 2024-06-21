cmake_minimum_required(VERSION 3.28)
project(server)

set(CMAKE_CXX_STANDARD 23)
set(WINDOWS_VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/windows)
set(LINUX_VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/linux)
set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(server src/main.cpp
        vendor/sqlite/sqlite3.c
        vendor/sqlite/sqlite_modern_cpp.h

        src/objects/base/base.cpp
        src/util/argument_parser.cpp
        src/util/io.cpp

        src/objects/root/init_cmd.cpp
        src/objects/root/receive.cpp
        src/objects/root/tasks.cpp

        src/objects/group/group.cpp
        src/objects/external/chatroom.cpp

        src/components/database/queries.cpp
        src/components/database/restore_objects.cpp
        src/components/database/restore_listeners.cpp
        src/components/database/fileman.cpp
        src/components/database/misc.cpp

        src/components/routing/endpoints.cpp
        src/components/routing/misc.cpp
        src/components/routing/websocket.cpp
        src/components/routing/handlers.cpp
        src/components/routing/listeners.cpp

        src/components/tree/send.cpp
        src/components/tree/misc.cpp

        src/components/instance/instance.cpp
        src/components/instance/misc.cpp
        src/components/instance/ssl.cpp
        src/components/instance/config.cpp
        src/components/eventlog/eventlog.cpp

        src/objects/agent/baphomet_receive.cpp
        src/objects/agent/baphomet_init_cmd.cpp
        src/objects/agent/baphomet_tasks_user.cpp
        src/objects/agent/baphomet_tasks_agent.cpp
        src/objects/agent/baphomet_misc.cpp
        src/objects/agent/baphomet_payloadgen.cpp

        include/argument_parser.hpp
        include/baphomet.hpp
        include/base.hpp
        include/chatroom.hpp
        include/common.hpp
        include/components.hpp
        include/enums.hpp
        include/group.hpp
        include/io.hpp
        include/macro.hpp
        include/root.hpp
        include/structs.hpp
        include/wintypes.hpp
        include/listeners.hpp
)

target_compile_definitions(server PRIVATE CROW_ENABLE_SSL)
target_include_directories(server PRIVATE ${MAIN_INCLUDE_DIR})

if(WIN32)
    target_compile_definitions(server PRIVATE WINDOWS)
    target_include_directories(server PRIVATE ${WINDOWS_VENDOR_DIR})
    target_link_directories(server PRIVATE ${WINDOWS_VENDOR_DIR})
    target_link_libraries(server PRIVATE libcrypto)
    target_link_libraries(server PRIVATE libssl)
    target_link_libraries(server PRIVATE crypt32)
    target_link_libraries(server PRIVATE ws2_32)
endif()

if(UNIX)
    target_compile_definitions(server PRIVATE LINUX)
    target_include_directories(server PRIVATE vendor/linux) #unfinished
endif()