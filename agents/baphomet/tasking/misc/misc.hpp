//
// Created by diago on 2024-05-27.
//

#ifndef MISC_HPP
#define MISC_HPP
#include <Windows.h>
#include <string>
#include <winternl.h>

namespace tasking {
    uint32_t    get_entry_point_rva(const std::string& file_buffer);
    void*       get_img_preferred_base(const std::string& file_buffer);
    void*       get_env_block();
    void        _RtlInitUnicodeString(PUNICODE_STRING UsStruct, PCWSTR Buffer);
}

#endif //MISC_HPP
