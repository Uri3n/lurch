//
// Created by diago on 2024-06-08.
//

#include <misc.hpp>

void*
tasking::remote_alloc(HANDLE hprocess, void *preferred, const uint32_t size, const uint32_t protect) {

    void* pages = VirtualAllocEx(
        hprocess,
        preferred,
        size,
        MEM_COMMIT | MEM_RESERVE,
        protect
    );

    return pages;
}


bool
tasking::remote_write(
        HANDLE hprocess,
        void* destination,
        void* source,
        const size_t size,
        const uint32_t protect_after
    ) {

    size_t bytes_written = 0;
    if(!WriteProcessMemory(
        hprocess,
        destination,
        source,
        size,
        &bytes_written
    ) || bytes_written != size) {
        return false;
    }


    if(protect_after != 0) {
        uint32_t old_protect = 0;
        return VirtualProtectEx(
            hprocess,
            destination,
            size,
            protect_after,
            reinterpret_cast<PDWORD>(&old_protect)
        ) == TRUE;
    }

    return true;
}


HANDLE
tasking::create_pagefile_backed_section(const uint32_t size, const uint32_t protect, NTSTATUS* out_status) {

    NTSTATUS          status            = 0;
    LARGE_INTEGER     li                = { 0 };
    fnNtCreateSection pcreate_section   = nullptr;
    HANDLE            hsection          = nullptr;

    //-----------------------------------------------------//

    pcreate_section = reinterpret_cast<decltype(pcreate_section)>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "NtCreateSection"));

    if(pcreate_section == nullptr) {
        return nullptr;
    }

    li.LowPart = size; //change to quadpart? lowpart might be incorrect here...
    status = pcreate_section(
        &hsection,
        SECTION_ALL_ACCESS,
        nullptr,
        &li,
        protect,
        SEC_COMMIT,
        nullptr
    );

    if(out_status != nullptr) {
        *out_status = status;
    }

    if(status != ERROR_SUCCESS) {
        return nullptr;
    }

    return hsection;
}


HANDLE
tasking::create_image_section(HANDLE himage, NTSTATUS* out_status) {

    fnNtCreateSection   pcreate_section = nullptr;
    HANDLE              hsection        = nullptr;
    NTSTATUS            status          = 0;

    //-----------------------------------------------------//

    pcreate_section = reinterpret_cast<decltype(pcreate_section)>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "NtCreateSection"));

    if(pcreate_section == nullptr) {
        return nullptr;
    }


    status = pcreate_section(
        &hsection,
        SECTION_ALL_ACCESS,
        nullptr,
        nullptr,
        PAGE_READONLY,
        SEC_IMAGE,
        himage
    );

    if(out_status != nullptr) {
        *out_status = status;
    }

    if(status != ERROR_SUCCESS) {
        return nullptr;
    }

    return hsection;
}

