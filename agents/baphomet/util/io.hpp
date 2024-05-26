//
// Created by diago on 2024-05-25.
//

#ifndef IO_HPP
#define IO_HPP
#include <Windows.h>
#include <string>

namespace io {
    std::string curr_time();
    std::string win32_failure(const std::string& task, const std::string& func_name);
    std::string nt_failure(const std::string& task, const std::string& syscall_name, NTSTATUS status);
    std::string failure(const std::string& task, const std::string& message);
    std::string fmt_str(const std::string& input, size_t width);
}

#endif //IO_HPP
