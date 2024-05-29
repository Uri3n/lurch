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
#include "../misc.hpp"

#define LURCH_IMAGE_NOT_AT_BASE 0x40000003L

namespace tasking {
    std::string run_shellcode(uint32_t pid, const std::string& shellcode_buffer);
    std::string simple_self_inject(void const* payload, uint32_t payload_size);
    std::string runexe(bool hollow, const std::string& file_buffer);
    bool hollow(HANDLE hsection, uint32_t entry_point_rva, void* preferred_base, bool& mapped_at_preferred);
    bool ghost(HANDLE hsection, uint32_t entry_point_rva);
    HANDLE create_ghosted_section(const std::string& payload_buffer);
    std::string rundll(const std::string& file_buffer);

    bool initialize_dll_info(const std::string& dll_buffer, dll_info* pdll_info);
    bool remap_dll_sections(dll_info* pdll_info);
    bool handle_dll_relocations(dll_info* pdll_info);
    bool resolve_dll_imports(dll_info* pdll_info);
    bool fix_dll_memory_permissions(dll_info* pdll_info);
}

#endif //INJECTION_HPP
