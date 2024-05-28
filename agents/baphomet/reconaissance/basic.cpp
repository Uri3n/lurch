//
// Created by diago on 2024-05-25.
//

#include "basic.hpp"

#include <iostream>


std::string recon::whoami() {

    PTOKEN_GROUPS_AND_PRIVILEGES    ptokeninfo       = nullptr;
    HANDLE                          htoken           = nullptr;
    fnRtlConvertSidToUnicodeString  pconversion_func = nullptr;
    wchar_t*                        sid_buffer       = nullptr;

    uint32_t account_name_buffer_size   = 0;
    uint32_t domain_name_buffer_size    = 0;
    uint32_t token_info_buffer_size     = 0;

    std::string account_name;
    std::string domain_name;
    std::string privilege_name;
    std::string result;

    NTSTATUS        status = 0;
    SID_NAME_USE    sid_type;
    UNICODE_STRING  us_sid = { 0 };

    auto _ = defer([&]() {
        FREE_HEAP_BUFFER(ptokeninfo);
        FREE_HEAP_BUFFER(sid_buffer);
        CLOSE_HANDLE(htoken);
    });


    sid_buffer = static_cast<wchar_t*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        72 * sizeof(wchar_t)
    ));

    if(sid_buffer == nullptr) {
        return io::win32_failure("whoami", "HeapAlloc");
    }

    us_sid.Buffer        = sid_buffer;
    us_sid.Length        = 72 * sizeof(wchar_t);
    us_sid.MaximumLength = 72 * sizeof(wchar_t);


    pconversion_func = reinterpret_cast<fnRtlConvertSidToUnicodeString>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "RtlConvertSidToUnicodeString"));

    if(pconversion_func == nullptr) {
        return io::win32_failure("whoami", "GetProcAddress");
    }


    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &htoken)) {
        return io::win32_failure("whoami", "OpenProcessToken");
    }

    GetTokenInformation(
        htoken,
        TokenGroupsAndPrivileges,
        nullptr,
        0,
        reinterpret_cast<PDWORD>(&token_info_buffer_size)
    );


    ptokeninfo = static_cast<PTOKEN_GROUPS_AND_PRIVILEGES>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        token_info_buffer_size
    ));

    if(ptokeninfo == nullptr) {
        return io::win32_failure("whoami", "HeapAlloc");
    }


    if(!GetTokenInformation(
        htoken,
        TokenGroupsAndPrivileges,
        ptokeninfo,
        token_info_buffer_size,
        reinterpret_cast<PDWORD>(&token_info_buffer_size)
    )) {
        return io::win32_failure("whoami", "GetTokenInformation");
    }

    //
    // enumerate SIDs.
    //

    result += "SECURITY IDENTIFIERS:\n\n";
    result += io::fmt_str("Name", 63);
    result += io::fmt_str("Type", 18);
    result += io::fmt_str("Value", 14);
    result += io::fmt_str("Attributes", 52);
    result += '\n';
    result += WHOAMI_SID_DIVIDING_CHARS;
    result += '\n';

    for(size_t i = 0; i < ptokeninfo->SidCount; i++) {

        SID_AND_ATTRIBUTES sid_and_attributes = ptokeninfo->Sids[i];
        status = pconversion_func(
            &us_sid,
            sid_and_attributes.Sid,
            FALSE
        );

        if(status != ERROR_SUCCESS) {
            return io::nt_failure("whoami", "RtlConvertSidToUnicodeString", status);
        }


        //
        // check req. buffer size first.
        //

        account_name_buffer_size = 0;
        domain_name_buffer_size  = 0;

        if(!LookupAccountSidA(
            nullptr,
            sid_and_attributes.Sid,
            nullptr,
            reinterpret_cast<LPDWORD>(&account_name_buffer_size),
            nullptr,
            reinterpret_cast<LPDWORD>(&domain_name_buffer_size),
            &sid_type
        ) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return io::win32_failure("whoami", "LookupAccountSidA");
        }

        account_name.resize(account_name_buffer_size);
        domain_name.resize(domain_name_buffer_size);


        //
        // get SID data for real this time.
        //

        if(!LookupAccountSidA(
            nullptr,
            sid_and_attributes.Sid,
            account_name.data(),
            reinterpret_cast<LPDWORD>(&account_name_buffer_size),
            domain_name.data(),
            reinterpret_cast<LPDWORD>(&domain_name_buffer_size),
            &sid_type
        )) {
            return io::win32_failure("whoami", "LookupAccountSidA");
        }

        if(account_name_buffer_size && domain_name_buffer_size) {
            std::string full_name = (domain_name.append("\\") + account_name);
            result += io::fmt_str(full_name, 65);
        }
        else {
            result += io::fmt_str("???", 63);
        }


        //
        // translate SID type.
        //

        switch(sid_type) {
            case SidTypeUser:
                result += io::fmt_str("User", 18);
                break;
            case SidTypeGroup:
                result += io::fmt_str("Group", 18);
                break;
            case SidTypeDomain:
                result += io::fmt_str("Domain", 18);
                break;
            case SidTypeAlias:
                result += io::fmt_str("Alias", 18);
                break;
            case SidTypeWellKnownGroup:
                result += io::fmt_str("Well-Known Group", 18);
                break;
            case SidTypeComputer:
                result += io::fmt_str("Computer", 18);
                break;
            case SidTypeDeletedAccount:
                result += io::fmt_str("Deleted Account", 18);
                break;
            case SidTypeLabel:
                result += io::fmt_str("Label", 18);
                break;
            case SidTypeLogonSession:
                result += io::fmt_str("Logon Session", 18);
                break;
            default:
                result += io::fmt_str("Unknown", 18);
                break;
        }


        std::string ansi_sid;
        for(size_t j = 0; us_sid.Buffer[j] != L'\0'; j++) {
            if(us_sid.Buffer[j] >= 0 && us_sid.Buffer[j] < 128) {
                ansi_sid += static_cast<char>(us_sid.Buffer[j]);
            } else {
                ansi_sid += "?";
            }
        }

        result += io::fmt_str(ansi_sid + ' ', 14);
        std::memset(us_sid.Buffer, 0, 72 * sizeof(wchar_t));


        //
        // SID attributes.
        //

        std::string attribute_str;
        if(sid_and_attributes.Attributes & SE_GROUP_ENABLED)
            attribute_str += "Enabled, ";

        if(sid_and_attributes.Attributes & SE_GROUP_ENABLED_BY_DEFAULT)
            attribute_str += "Enabled By Default, ";

        if(sid_and_attributes.Attributes & SE_GROUP_INTEGRITY)
            attribute_str += "Integrity, ";

        if(sid_and_attributes.Attributes & SE_GROUP_LOGON_ID)
            attribute_str += "Logon ID, ";

        if(sid_and_attributes.Attributes & SE_GROUP_MANDATORY)
            attribute_str += "Mandatory, ";

        if(sid_and_attributes.Attributes & SE_GROUP_OWNER)
            attribute_str += "Owner, ";

        if(sid_and_attributes.Attributes & SE_GROUP_RESOURCE)
            attribute_str += "Resource, ";

        if(sid_and_attributes.Attributes & SE_GROUP_USE_FOR_DENY_ONLY)
            attribute_str += "Deny Only, ";

        if(attribute_str.size() >= 2) {
            attribute_str.erase(attribute_str.size() - 2);
        }
        else {
            attribute_str += "No Attributes.";
        }

        result += io::fmt_str(attribute_str, 52);
        result += '\n';
    }


    result += "\n\nPRIVILEGES INFORMATION:\n";
    result += io::fmt_str("Name", 30);
    result += io::fmt_str("State", 9);
    result += '\n';
    result += WHOAMI_PRIVS_DIVIDING_CHARS;
    result += '\n';

    for(size_t i = 0; i < ptokeninfo->PrivilegeCount; i++) {
        LUID luid = ptokeninfo->Privileges[i].Luid;
        uint32_t buffer_size = 0;

        LookupPrivilegeNameA( //get correct buffer size
            nullptr,
            &luid,
            nullptr,
            reinterpret_cast<LPDWORD>(&buffer_size)
        );

        privilege_name.resize(buffer_size);
        if(!LookupPrivilegeNameA(
            nullptr,
            &luid,
            privilege_name.data(),
            reinterpret_cast<LPDWORD>(&buffer_size)
        )) {
            return io::win32_failure("whoami", "LookupPrivilegeNameA");
        }

        result += io::fmt_str(privilege_name, 31);
        if(ptokeninfo->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) {
            result += io::fmt_str("Enabled", 9);
        } else {
            result += io::fmt_str("Disabled", 9);
        }

        result += '\n';
    }

    return result;
}


bool
recon::get_integrity_level(std::string& integrity_level_str) {

    HANDLE                  htoken = nullptr;
    PTOKEN_MANDATORY_LABEL  plabel = nullptr;
    uint32_t                length_needed   = 0;
    uint32_t                integrity_level = 0;

    auto _ = defer([&]() {
        CLOSE_HANDLE(htoken);
        FREE_HEAP_BUFFER(plabel);
    });


    if(!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_QUERY,
        &htoken
    )) {
        return false;
    }

    if(!GetTokenInformation(
        htoken,
        TokenIntegrityLevel,
        nullptr,
        0,
        reinterpret_cast<LPDWORD>(&length_needed)
    ) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return false;
    }


    plabel = static_cast<PTOKEN_MANDATORY_LABEL>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        length_needed
    ));

    if(plabel == nullptr) {
        return false;
    }

    if(!GetTokenInformation(
        htoken,
        TokenIntegrityLevel,
        plabel,
        length_needed,
        reinterpret_cast<LPDWORD>(&length_needed)
    )) {
        return false;
    }


    integrity_level = *GetSidSubAuthority(plabel->Label.Sid, (*GetSidSubAuthorityCount(plabel->Label.Sid) - 1));
    if(     integrity_level >= SECURITY_MANDATORY_SYSTEM_RID) {
        integrity_level_str = "system";
    }
    else if(integrity_level >= SECURITY_MANDATORY_HIGH_RID ) {
        integrity_level_str = "high";
    }
    else if(integrity_level >= SECURITY_MANDATORY_MEDIUM_RID) {
        integrity_level_str = "medium";
    }
    else if(integrity_level >= SECURITY_MANDATORY_LOW_RID ) {
        integrity_level_str = "low";
    }
    else {
        integrity_level_str = "unknown";
    }

    return true;
}

bool
recon::get_user_sid(std::string &sid) {

    fnRtlConvertSidToUnicodeString  pconversion_func = nullptr;
    PTOKEN_USER                     ptoken_user = nullptr;
    UNICODE_STRING                  us_sid;
    uint32_t                        buffer_size = 0;
    NTSTATUS                        status      = 0;
    HANDLE                          htoken      = nullptr;
    wchar_t*                        sid_buffer  = nullptr;

    auto _ = defer([&]() {
        FREE_HEAP_BUFFER(ptoken_user);
        FREE_HEAP_BUFFER(sid_buffer);
        CLOSE_HANDLE(htoken);
    });


    sid_buffer = static_cast<wchar_t*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        72 * sizeof(wchar_t) //maximum size of an SID including the null term (i think).
    ));

    if(sid_buffer == nullptr) {
        return false;
    }

    us_sid.Buffer           = sid_buffer;
    us_sid.Length           = 72 * sizeof(wchar_t);
    us_sid.MaximumLength    = 72 * sizeof(wchar_t);


    pconversion_func = reinterpret_cast<fnRtlConvertSidToUnicodeString>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "RtlConvertSidToUnicodeString"));

    if(pconversion_func == nullptr) {
        return false;
    }


    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &htoken)) {
        return false;
    }

    if(!GetTokenInformation(
        htoken,
        TokenUser,
        nullptr,
        0,
        reinterpret_cast<LPDWORD>(&buffer_size)
    ) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return false;
    }

    ptoken_user = static_cast<PTOKEN_USER>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        buffer_size
    ));

    if(ptoken_user == nullptr) {
        return false;
    }

    if(!GetTokenInformation(
        htoken,
        TokenUser,
        ptoken_user,
        buffer_size,
        reinterpret_cast<LPDWORD>(&buffer_size)
    )) {
        return false;
    }


    status = pconversion_func(
        &us_sid,
        ptoken_user->User.Sid,
        FALSE
    );

    if(status != ERROR_SUCCESS) {
        return false;
    }


    for(size_t i = 0; us_sid.Buffer[i] != L'\0'; i++) {
        if(us_sid.Buffer[i] >= 0 && us_sid.Buffer[i] < 128) {
            sid += static_cast<char>(us_sid.Buffer[i]);
        }
        else {
            sid += "?";
        }
    }

    return true;
}


bool
recon::get_logged_on_user(std::string &user) {

    char*       heap_buffer             = nullptr;
    const char  environment_variable[]  = { "USERNAME" };

    auto _ = defer([&]() {
       FREE_HEAP_BUFFER(heap_buffer);
    });


    heap_buffer = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        500 //hardcoded, but should be more than sufficient.
    ));

    if(heap_buffer == nullptr) {
        return false;
    }

    if(!GetEnvironmentVariableA(
        environment_variable,
        heap_buffer,
        500
    )) {
        return false;
    }

    user = std::string(heap_buffer);
    return true;
}


bool
recon::get_desktop_name(std::string &desktop_name) {

    HDESK hdesktop = GetThreadDesktop(GetCurrentThreadId());
    if(hdesktop == nullptr) {
        return false;
    }

    char buffer[MAX_PATH]   = { 0 };
    uint32_t length_needed  = 0;

    if(!GetUserObjectInformationA(
        hdesktop,
        UOI_NAME,
        buffer,
        MAX_PATH,
        reinterpret_cast<LPDWORD>(&length_needed)
    )) {
        return false;
    }

    desktop_name = buffer;
    return true;
}


bool
recon::get_host_name(std::string &host_name) {

    uint32_t required_size = 0;
    if(!GetComputerNameA(
        nullptr,
        reinterpret_cast<LPDWORD>(&required_size)
    ) && GetLastError() != ERROR_BUFFER_OVERFLOW) {
        return false;
    }

    host_name.resize(required_size);
    return static_cast<bool>(GetComputerNameA(
        host_name.data(),
        reinterpret_cast<LPDWORD>(&required_size)
    ));
}


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


