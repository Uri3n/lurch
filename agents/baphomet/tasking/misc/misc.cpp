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
