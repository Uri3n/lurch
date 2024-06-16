//
// Created by diago on 2024-06-15.

#include <anti_analysis.hpp>


bool
anti_analysis::delete_self() {

    FILE_DISPOSITION_INFO   dispos = { 0 };
    PFILE_RENAME_INFO       rename = { 0 };
    HANDLE                  hfile  = nullptr;

    wchar_t                 curr_path[MAX_PATH * 2] = { 0 };
    wchar_t                 newstream[]             = { L":Random" };
    size_t                  streamlen               = wcslen(newstream) * sizeof(wchar_t);
    bool                    state                   = false;

    //-----------------------------------------------------------------//

    auto _ = defer([&]() {
       if(!state) {
           if(rename != nullptr) {
               HeapFree(GetProcessHeap(), 0, rename);
           } if(hfile != INVALID_HANDLE_VALUE && hfile != nullptr) {
               CloseHandle(hfile);
           }
       }
    });


    //
    // allocate heap buffer for rename structure
    //

    rename = static_cast<decltype(rename)>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(FILE_RENAME_INFO) + streamlen
    ));

    if(rename == nullptr) {
        return state;
    }


    dispos.DeleteFile       = TRUE;
    rename->FileNameLength  = streamlen;
    memcpy(rename->FileName, newstream, streamlen);


    //
    // get a handle to the file this process was created from
    //

    if(GetModuleFileNameW(nullptr, curr_path, MAX_PATH * 2) == 0) {
        return state;
    }

    hfile = CreateFileW(
        curr_path,
        DELETE | SYNCHRONIZE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if(hfile == INVALID_HANDLE_VALUE) {
        DEBUG_PRINT("[!] CreateFileW(1) failed with: %lu\n", GetLastError());
        return state;
    }


    //
    // change main data stream name
    //

    if(!SetFileInformationByHandle(
        hfile,
        FileRenameInfo,
        rename,
        sizeof(FILE_RENAME_INFO) + streamlen
    )) {
        DEBUG_PRINT("[!] SetFileInformationByHandle(1) failed with: %lu\n", GetLastError());
        return state;
    }
    DEBUG_PRINT("[+] Changed main data stream name.\n");


    //
    // reopen file under new data stream name and mark it for deletion
    //

    CloseHandle(hfile);
    hfile = CreateFileW(
        curr_path,
        DELETE | SYNCHRONIZE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if(hfile == INVALID_HANDLE_VALUE) {
        DEBUG_PRINT("[!] CreateFileW(2) failed with: %lu\n", GetLastError());
        return state;
    }

    if(!SetFileInformationByHandle(hfile, FileDispositionInfo, &dispos, sizeof(dispos))) {
        DEBUG_PRINT("[!] SetFileInformationByHandle(2) failed with: %lu\n", GetLastError());
        return state;
    }


    DEBUG_PRINT("[+] goodbye.\n");
    CloseHandle(hfile);

    state = true;
    return state;
}


bool
anti_analysis::debug_check_1() {
    DEBUG_PRINT("[+] Debug check 1\n");
    return IsDebuggerPresent();
}

bool
anti_analysis::debug_check_2() {
    DEBUG_PRINT("[+] Debug check 2\n");
    PPEB ppeb = (PPEB)__readgsqword(0x60);
    return ppeb->BeingDebugged == 1;
}

bool
anti_analysis::debug_check_3() {
    DEBUG_PRINT("[+] Debug check 3\n");
    PUNDOC_PEB ppeb = (PUNDOC_PEB)__readgsqword(0x60);
    return ppeb->NtGlobalFlag == (FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS);
}

bool
anti_analysis::debug_check_4() {
    DEBUG_PRINT("[+] Debug check 4\n");

    NTSTATUS                    status              = 0;
    uint64_t                    is_debugger_present = 0;
    uint64_t                    proc_debug_object   = 0;
    fnNtQueryInformationProcess pquery_process      = nullptr;

    //----------------------------------------------------------//

    if(!tasking::get_function_ptr(pquery_process, "NTDLL.DLL", "NtQueryInformationProcess")) {
        return false;
    }


    //
    // First method, via debug port
    //

    status = pquery_process(
        GetCurrentProcess(),
        ProcessDebugPort,
        &is_debugger_present,
        sizeof(is_debugger_present),
        nullptr
    );

    if(status != ERROR_SUCCESS) {
        DEBUG_PRINT("[!] NtQueryInformationProcess(1): 0x%0.8X\n", status);
        return false;
    }

    if(is_debugger_present != 0) {
        return true;
    }


    //
    // Second method, via debug object handle
    //

    status = pquery_process(
        GetCurrentProcess(),
        (PROCESSINFOCLASS)30,   // ProcessDebugObjectHandle
        &proc_debug_object,
        sizeof(proc_debug_object),
        nullptr
    );

    if(status != ERROR_SUCCESS && status != 0xC0000353) {
        DEBUG_PRINT("[!] NtQueryInformationProcess(2): 0x%0.8X\n", status);
        return false;
    }

    return proc_debug_object != 0;
}

bool
anti_analysis::debug_check_5() {
    DEBUG_PRINT("[+] Debug check 5\n");

    CONTEXT ctx      = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    if(!GetThreadContext(GetCurrentThread(), &ctx)) {
        DEBUG_PRINT("[!] GetThreadContext: %lu\n", GetLastError());
        return false;
    }

    return ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0;
}

bool
anti_analysis::being_debugged() {

    return  debug_check_1() ||
            debug_check_2() ||
            debug_check_3() ||
            debug_check_4() ||
            debug_check_5();
}
