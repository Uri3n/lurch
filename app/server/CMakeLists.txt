cmake_minimum_required(VERSION 3.28)
project(server)

set(CMAKE_CXX_STANDARD 23)

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
        components/instance.cpp
        components/instance.hpp
        components/database.cpp
        components/router.cpp
        components/object_tree.cpp
        objects/agent/agent.cpp
        objects/agent/agent.hpp
        objects/root/root.cpp
        objects/root/root.hpp
        objects/group/group.cpp
        objects/group/group.hpp
)
