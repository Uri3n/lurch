cmake_minimum_required(VERSION 3.28)
project(server)

set(CMAKE_CXX_STANDARD 23)
set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UNIX)
    message(FATAL_ERROR "This project is technically buildable on Linux because all dependencies are cross-platform.\n
                         However, I haven't figured everything out yet.")
endif()

add_executable(server src/main.cpp

        vendor/sqlite/sqlite3.c
        vendor/sqlite/sqlite_modern_cpp.h

        src/objects/base/base.cpp
        src/util/argument_parser.cpp
        src/util/io.cpp
        src/util/templating.cpp

        src/objects/root/root_init_cmd.cpp
        src/objects/root/root_receive.cpp
        src/objects/root/root_tasks.cpp

        src/objects/group/genericgroup_init_cmd.cpp
        src/objects/group/genericgroup_tasks.cpp
        src/objects/group/genericgroup_receive.cpp
        src/objects/external/chatroom.cpp

        src/components/database/restore_objects.cpp
        src/components/database/restore_listeners.cpp
        src/components/database/fileman.cpp
        src/components/database/misc.cpp
        src/components/database/queries_store.cpp
        src/components/database/queries_get.cpp
        src/components/database/queries_delete.cpp

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
        include/templating.hpp


)


# OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(server PRIVATE OpenSSL::SSL)
target_link_libraries(server PRIVATE OpenSSL::Crypto)


# CrowCPP
#   - This will only work on Linux.
#   - If on Windows, crow will need to be in Path via vcpkg or other.
if(UNIX)
    find_package(Crow REQUIRED)
    target_link_libraries(server PRIVATE Crow::Crow)
endif()


target_compile_definitions(server PRIVATE CROW_ENABLE_SSL)
target_include_directories(server PRIVATE ${MAIN_INCLUDE_DIR})

if(WIN32)
    target_compile_definitions(server PRIVATE WINDOWS)
    target_link_libraries(server PRIVATE crypt32)       # required for OpenSSL
    target_link_libraries(server PRIVATE ws2_32)        # required for OpenSSL
endif()
