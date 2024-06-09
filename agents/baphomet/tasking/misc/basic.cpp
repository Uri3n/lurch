//
// Created by diago on 2024-05-27.
//

#include "misc.hpp"

void*
tasking::get_env_block() {

    // don't even ask...just don't.
    PPEB ppeb = (PPEB)__readgsqword(0x60);
    uint8_t* process_params = reinterpret_cast<uint8_t*>(ppeb->ProcessParameters);

    process_params += 0x80;
    return (void*)(*((void**)process_params));
}


uint32_t
tasking::rva_of(
        _In_ HMODULE module_ptr,
        _In_ const std::string &func_name,
        _Out_opt_ void** func_address
    ) {

    unsigned char* base = reinterpret_cast<decltype(base)>(module_ptr);

    PIMAGE_DOS_HEADER       pdos_hdr    = nullptr;
    PIMAGE_NT_HEADERS       pnt_hdrs    = nullptr;
    PIMAGE_EXPORT_DIRECTORY pexport_dir = nullptr;

    uint32_t*               pnames      = nullptr;
    uint32_t*               paddresses  = nullptr;
    uint16_t*               pordinals   = nullptr;


    //
    // init
    //

    if(base == nullptr) {
        return 0;
    }

    pdos_hdr = reinterpret_cast<decltype(pdos_hdr)>(module_ptr);
    if(pdos_hdr->e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    pnt_hdrs = reinterpret_cast<decltype(pnt_hdrs)>(base + pdos_hdr->e_lfanew);
    if(pnt_hdrs->Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    //
    // exports
    //

    const uint32_t export_rva = pnt_hdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    pexport_dir = reinterpret_cast<decltype(pexport_dir)>(base + export_rva);

    pnames      = reinterpret_cast<uint32_t*>(base + pexport_dir->AddressOfNames);
    paddresses  = reinterpret_cast<uint32_t*>(base + pexport_dir->AddressOfFunctions);
    pordinals   = reinterpret_cast<uint16_t*>(base + pexport_dir->AddressOfNameOrdinals);

    for(size_t i = 0; i < pexport_dir->NumberOfFunctions; i++) {

        char* name      = reinterpret_cast<char*>(base + pnames[i]);
        void* address   = base + paddresses[pordinals[i]];

        if(func_name == name) {
            if(func_address != nullptr) {
                *func_address = address;
            }
            return paddresses[pordinals[i]];
        }
    }

    return 0;
}


uint32_t
tasking::get_entry_point_rva(const std::string &file_buffer) {

    if (file_buffer.empty()) {
        return 0;
    }

    PIMAGE_DOS_HEADER pimage_dos_header = (PIMAGE_DOS_HEADER)file_buffer.data();
    if (pimage_dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    PIMAGE_NT_HEADERS pimage_nt_headers = (PIMAGE_NT_HEADERS)(file_buffer.data() + pimage_dos_header->e_lfanew);
    if (pimage_nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    return pimage_nt_headers->OptionalHeader.AddressOfEntryPoint;
}


void*
tasking::get_img_preferred_base(const std::string &file_buffer) {

    if(file_buffer.empty()) {
        return nullptr;
    }

    PIMAGE_NT_HEADERS pimage_nt_headers =
        (PIMAGE_NT_HEADERS)(file_buffer.data() + ((PIMAGE_DOS_HEADER)file_buffer.data())->e_lfanew);

    if(pimage_nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        return nullptr;
    }

    return reinterpret_cast<void*>(pimage_nt_headers->OptionalHeader.ImageBase);
}


void
tasking::_RtlInitUnicodeString(PUNICODE_STRING UsStruct, PCWSTR Buffer) {

    if ((UsStruct->Buffer = (PWSTR)Buffer)) {

        unsigned int Length = wcslen(Buffer) * sizeof(WCHAR);
        if (Length > 0xfffc) {
            Length = 0xfffc;
        }

        UsStruct->Length = Length;
        UsStruct->MaximumLength = UsStruct->Length + sizeof(WCHAR);
    }

    else UsStruct->Length = UsStruct->MaximumLength = 0;
}


HANDLE
tasking::write_into_file(
        const std::string &buff,
        const std::string &file_name,
        const uint32_t bytes_to_write,
        const bool delete_on_close
    ) {

    HANDLE hfile = CreateFileA(
        file_name.c_str(),
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


    if(delete_on_close) {
        FILE_DISPOSITION_INFO dispos = { 0 };
        dispos.DeleteFile = TRUE;

        if(!SetFileInformationByHandle(
            hfile,
            FileDispositionInfo,
            &dispos,
            sizeof(dispos)
        )) {
            CloseHandle(hfile);
            return nullptr;
        }
    }


    uint32_t bytes_written = 0;
    if(!WriteFile(
        hfile,
        buff.data(),
        bytes_to_write,
        reinterpret_cast<LPDWORD>(&bytes_written),
        nullptr
    ) || bytes_written != bytes_to_write) {
        CloseHandle(hfile);
        return nullptr;
    }

    return hfile;
}


bool
tasking::create_anonymous_pipe(_Out_ HANDLE* hread, _Out_ HANDLE* hwrite) {

    SECURITY_ATTRIBUTES sa  = { 0 };
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = TRUE;

    if(!CreatePipe(hread, hwrite, &sa, 0)) {
        return false;
    }

    return true;
}
