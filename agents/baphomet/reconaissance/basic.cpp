//
// Created by diago on 2024-05-25.
//

#include "basic.hpp"


bool
recon::get_current_process_name(_Out_ std::string &process_name) {

    char module_filename[MAX_PATH] = { 0 };
    size_t base_name_start = 0;

    if(!GetModuleFileNameA(nullptr, module_filename, MAX_PATH)) {
        return false;
    }

    for(size_t i = 0; module_filename[i] != '\0'; i++) {
        if(module_filename[i] == '\\') {
            base_name_start = i;
        }
    }

    process_name = std::string(module_filename + base_name_start + 1);
    return true;
}

uint32_t
recon::get_current_process_pid() {
    return GetCurrentProcessId();
}

uint32_t
recon::get_major_os_version() {
    PUNDOC_PEB pPeb = (PUNDOC_PEB)__readgsqword(0x60);
    return pPeb->OSMajorVersion;
}



