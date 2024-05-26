//
// Created by diago on 2024-05-25.
//

#include "shell.hpp"

#include <iostream>


std::string
tasking::shell_command(std::string usr_arg_string, const bool powershell) {

    std::string output;

    char* pipe_buffer   = nullptr;
    char* non_const     = nullptr;

    STARTUPINFOA        startup_info        = { 0 };
    PROCESS_INFORMATION proc_info           = { 0 };
    SECURITY_ATTRIBUTES sec_attr            = { 0 };

    HANDLE hPipe_write  = nullptr;
    HANDLE hPipe_read   = nullptr;

    auto _ = defer([&]() {
        FREE_HEAP_BUFFER(pipe_buffer);
        FREE_HEAP_BUFFER(non_const);
        CLOSE_HANDLE(hPipe_read);
        CLOSE_HANDLE(hPipe_write);
        CLOSE_HANDLE(proc_info.hThread);
        CLOSE_HANDLE(proc_info.hProcess);
    });


    if(usr_arg_string.empty()) {
        return io::failure((powershell ? "ps" : "cmd"), "no command line args given.");
    }

    if(powershell) {
        usr_arg_string.insert(0, "powershell.exe ");
    }


    //
    // allocate heap buffers
    //

    pipe_buffer = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        4096
    ));

    non_const = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        usr_arg_string.size() + 1
    ));

    if(non_const == nullptr || pipe_buffer == nullptr) {
        return io::win32_failure((powershell ? "ps" : "cmd"), "HeapAlloc");
    }

    std::memcpy(non_const, usr_arg_string.c_str(), usr_arg_string.size());


    //
    // create pipe
    //

    sec_attr.bInheritHandle         = TRUE;
    sec_attr.lpSecurityDescriptor   = nullptr;
    sec_attr.nLength                = sizeof(SECURITY_ATTRIBUTES);

    if(!CreatePipe(
        &hPipe_read,
        &hPipe_write,
        &sec_attr,
        0
    )) {
        return io::win32_failure((powershell ? "ps" : "cmd"), "CreatePipe");
    }


    //
    // create child process and read stdout
    //

    startup_info.cb = sizeof(startup_info);
    startup_info.hStdError = hPipe_write;
    startup_info.hStdOutput = hPipe_write;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;

    if(!CreateProcessA(
        nullptr,
        non_const,
        nullptr,
        nullptr,
        TRUE,
        (powershell ? 0 : CREATE_NO_WINDOW),
        nullptr,
        nullptr,
        &startup_info,
        &proc_info
    )) {
        return io::win32_failure((powershell ? "ps" : "cmd"), "CreateProcessA");
    }

    CloseHandle(hPipe_write);
    hPipe_write = nullptr;

    uint32_t bytes_read = 0;
    while(ReadFile(
        hPipe_read,
        pipe_buffer,
        4095,
        reinterpret_cast<LPDWORD>(&bytes_read),
        nullptr
    ) && bytes_read > 0) {
        output += pipe_buffer;
        std::memset(pipe_buffer, '\0', 4096);
    }

    if(output.empty()) {
        return "successfully executed command, no output.";
    }

    return output;
}
