//
// Created by diago on 2024-05-27.
//

#ifndef INJECTION_HPP
#define INJECTION_HPP
#include <Windows.h>
#include <string>
#include "../../util/common.hpp"
#include "../../util/io.hpp"
#include "../../util/function_ptrs.hpp"
#include "../../util/structs.hpp"

namespace tasking {
    std::string run_shellcode(uint32_t pid, const std::string& shellcode_buffer);
    std::string simple_self_inject(void const* payload, uint32_t payload_size);
    std::string runexe(bool hollow, const std::string& file_buffer);
    HANDLE create_ghosted_section(const std::string& payload_buffer);
}

#endif //INJECTION_HPP
