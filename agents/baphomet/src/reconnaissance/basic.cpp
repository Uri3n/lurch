//
// Created by diago on 2024-05-25.
//

#include <reconnaissance.hpp>


std::string
recon::get_sid_group_attributes(const uint32_t attr_mask) {

    std::string attribute_str;

    if(attr_mask & SE_GROUP_ENABLED)
        attribute_str += "Enabled, ";

    if(attr_mask & SE_GROUP_ENABLED_BY_DEFAULT)
        attribute_str += "Enabled By Default, ";

    if(attr_mask & SE_GROUP_INTEGRITY)
        attribute_str += "Integrity, ";

    if(attr_mask & SE_GROUP_LOGON_ID)
        attribute_str += "Logon ID, ";

    if(attr_mask & SE_GROUP_MANDATORY)
        attribute_str += "Mandatory, ";

    if(attr_mask & SE_GROUP_OWNER)
        attribute_str += "Owner, ";

    if(attr_mask & SE_GROUP_RESOURCE)
        attribute_str += "Resource, ";

    if(attr_mask & SE_GROUP_USE_FOR_DENY_ONLY)
        attribute_str += "Deny Only, ";

    if(attribute_str.size() >= 2) {
        attribute_str.erase(attribute_str.size() - 2);
    }
    else {
        attribute_str += "No Attributes.";
    }

    return attribute_str;
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


    sid.clear();
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
recon::is_domain_joined(std::string& domain_name) {

    wchar_t*                buff = nullptr;
    NETSETUP_JOIN_STATUS    join_status;
    NET_API_STATUS          api_status = 0;

    auto _ = defer([&]() {
        if(buff != nullptr) {
            NetApiBufferFree(buff);
        }
    });

    domain_name.clear();
    api_status = NetGetJoinInformation(nullptr, &buff, &join_status);
    if(api_status != NERR_Success) {
        return false;
    }

    if(join_status == NetSetupDomainName) {

        const std::wstring ws = buff;
        for(size_t i = 0; i < ws.size(); i++ ) {
            if(ws[i] >= 0 && ws[i] < 128) {
                domain_name += static_cast<char>(ws[i]);
            } else {
                domain_name += '?';
            }
        }

        return true;
    }

    return false;
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


std::string
recon::generate_basic_info() {

    std::string     buff;
    std::string     output;
    const uint32_t  pid            = get_current_process_pid();
    const uint32_t  major_os_ver   = get_major_os_version();

    output += io::fmt_str("process name: ", 22)             + (get_current_process_name(buff) ? buff : "?")                         + '\n';
    output += io::fmt_str("PID: ", 22)                      + (!pid ? "?" : std::to_string(pid))                                    + '\n';
    output += io::fmt_str("Integrity Level: ", 22)          + (!get_integrity_level(buff) ? "?" : buff)                             + '\n';
    output += io::fmt_str("Operating System: ", 22)         + (!major_os_ver ? "?" : ("Windows " + std::to_string(major_os_ver)))   + '\n';
    output += io::fmt_str("Logged On User: ",22)            + (!get_logged_on_user(buff) ? "?" : buff)                              + '\n';
    output += io::fmt_str("SID: ",22)                       + (!get_user_sid(buff) ? "?" : buff)                                    + '\n';
    output += io::fmt_str("Host: ",22)                      + (!get_host_name(buff) ? "?" : buff)                                   + '\n';
    output += io::fmt_str("Desktop Name: ",22)              + (!get_desktop_name(buff) ? "?" : buff)                                + '\n';
    output += io::fmt_str("Domain-Joined: ",22)             + (!is_domain_joined(buff) ? "No" : "Yes")                              + '\n';
    output += io::fmt_str("Domain Name: ",22)               + (buff.empty() ? "N/A" : buff)                                         + '\n';

    return output;
}
