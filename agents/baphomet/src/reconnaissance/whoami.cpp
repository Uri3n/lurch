//
// Created by diago on 2024-06-10.
//

#include <reconnaissance.hpp>

std::string recon::whoami() {

    PTOKEN_GROUPS_AND_PRIVILEGES    ptokeninfo       = nullptr;
    HANDLE                          htoken           = nullptr;
    fnRtlConvertSidToUnicodeString  pconversion_func = nullptr;
    wchar_t*                        sid_buffer       = nullptr;

    uint32_t        account_name_buffer_size   = 0;
    uint32_t        domain_name_buffer_size    = 0;
    uint32_t        token_info_buffer_size     = 0;

    std::string     account_name;
    std::string     domain_name;
    std::string     privilege_name;
    std::string     result;

    NTSTATUS        status = 0;
    SID_NAME_USE    sid_type;
    UNICODE_STRING  us_sid = { 0 };

    //------------------------------------------------------------//

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

    for(size_t i = 0; i < ptokeninfo->SidCount; i++) {

        if(i == 0) {
            result += io::fmt_str("Username", 24);
            result += io::fmt_str("SID", 45);
            result += '\n';
            result += WHOAMI_USERNAME_DIVIDING_CHARS;
            result += '\n';
        }

        if(i == 1) {
            result += io::fmt_str("Name", 63);
            result += io::fmt_str("SID", 18);
            result += io::fmt_str("Value", 14);
            result += io::fmt_str("Attributes", 52);
            result += '\n';
            result += WHOAMI_SID_DIVIDING_CHARS;
            result += '\n';
        }

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
            std::string full_name = (domain_name.append("\\") + account_name);              // Name
            result += io::fmt_str(full_name, i == 0 ? 26 : 65);
        }
        else {
            result += io::fmt_str("Well-Known SID", 63);
        }


        if(i > 0) {
            result += io::fmt_str(get_sid_type(sid_type),18);                               // Type
        }

        std::string ansi_sid;
        for(size_t j = 0; us_sid.Buffer[j] != L'\0'; j++) {
            if(us_sid.Buffer[j] >= 0 && us_sid.Buffer[j] < 128) {
                ansi_sid += static_cast<char>(us_sid.Buffer[j]);                            // SID
            } else {
                ansi_sid += "?";
            }
        }

        result += io::fmt_str(ansi_sid + ' ', i == 0 ? 45 : 14);
        memset(us_sid.Buffer, 0, 72 * sizeof(wchar_t));

        if(i == 0) {                                                                        // In case of first iteration, continue.
            result += "\n\n";
            continue;
        }


        result += io::fmt_str(get_sid_group_attributes(sid_and_attributes.Attributes), 52); // Attributes.
        result += '\n';
    }


    //
    // Get privilege information and state.
    //

    result += "\n\nPRIVILEGES INFORMATION:\n\n";
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


std::string
recon::get_sid_type(const SID_NAME_USE sid_type) {

    switch(sid_type) {
        case SidTypeUser:
            return "User";
        case SidTypeGroup:
            return "Group";
        case SidTypeDomain:
            return "Domain";
        case SidTypeAlias:
            return "Alias";
        case SidTypeWellKnownGroup:
            return "Well-Known Group";
        case SidTypeComputer:
            return "Computer";
        case SidTypeDeletedAccount:
            return "Deleted Account";
        case SidTypeLabel:
            return "Label";
        case SidTypeLogonSession:
            return "Logon Session";
        default:
            return "Unknown";
    }
}