//
// Created by diago on 2024-05-25.
//

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP
#include <windows.h>
#include <string>
#include <io.hpp>

namespace tasking {
    std::string pwd();
    std::string ls();
    std::string cd(const std::string& directory);
    std::string cat(const std::string& file_name);
    std::string mkdir(const std::string& directory_name);
    std::string cp(const std::string& source, const std::string& destination);
    std::string rm(const std::string& directory_entry);
    HANDLE get_file_handle(const std::string& directory_entry);
}

#endif //FILESYSTEM_HPP
