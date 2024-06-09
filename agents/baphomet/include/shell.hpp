//
// Created by diago on 2024-05-25.
//

#ifndef SHELL_HPP
#define SHELL_HPP
#include <Windows.h>
#include <string>
#include <common.hpp>
#include <io.hpp>

namespace tasking {
    std::string shell_command(std::string arg_string, bool powershell);
}

#endif //SHELL_HPP
