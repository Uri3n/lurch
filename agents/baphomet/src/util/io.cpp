//
// Created by diago on 2024-05-25.
//

#include <io.hpp>
#include <iostream>

std::string
io::curr_time() {
    SYSTEMTIME sys_time = { 0 };
    GetSystemTime(&sys_time);

    return std::to_string(sys_time.wHour)       + ':' +
            std::to_string(sys_time.wMinute)    + ':' +
            std::to_string(sys_time.wSecond)    + '\n';
}

std::string
io::failure(const std::string& task, const std::string& message) {

    std::string res = "Task " + task + " failed." + '\n';
    res += "  " + message + '\n';
    res += "  Failure time (UTC): " + curr_time() + '\n';
    return res;
}

std::string
io::win32_failure(const std::string& task, const std::string& func_name) {

    std::string res = "Task " + task + " failed." + '\n';
    res += "  Failed function call " + func_name + " status code (WIN32): " + std::to_string(GetLastError()) + '\n';
    res += "  Failure time (UTC): " + curr_time() + '\n';
    return res;
}

std::string
io::nt_failure(const std::string& task, const std::string& syscall_name, const NTSTATUS status ) {

    std::string hex_str;
    hex_str.resize(1024);
    wsprintfA(hex_str.data(), "0x%X", status);
    hex_str.shrink_to_fit();

    std::string res = "Task " + task + " failed." + '\n';
    res += "  Failed syscall " + syscall_name + " status code (NTSTATUS): " + hex_str + '\n';
    res += "  Failure time (UTC): " + curr_time() + '\n';
    return res;
}

std::string
io::fmt_str(const std::string& input, const size_t width) {

    std::string output;
    if(width) {
        output.resize(width);
        std::memset(output.data(), ' ', output.size());
    }

    if(input.size() >= width) {
        return input;
    }

    for(size_t i = 0; i < input.size(); i++) {
        if(i >= width) {
            output += input[i];
        }
        else {
            output[i] = input[i];
        }
    }

    return output;
}




