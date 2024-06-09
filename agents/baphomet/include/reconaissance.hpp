//
// Created by diago on 2024-06-04.
//

#ifndef RECONAISSANCE_HPP
#define RECONAISSANCE_HPP
#include <Windows.h>
#include <winternl.h>
#include <string>
#include <common.hpp>
#include <function_ptrs.hpp>
#include <io.hpp>
#include <structs.hpp>
#include <macro.hpp>

namespace recon {
    bool        get_desktop_name(std::string& desktop_name);
    bool        get_host_name(std::string& host_name);
    bool        get_logged_on_user(std::string& user);
    bool        get_current_process_name(std::string& process_name);
    bool        get_user_sid(std::string& sid);
    bool        get_integrity_level(std::string& integrity_lvl_str);
    uint32_t    get_current_process_pid();
    uint32_t    get_major_os_version();
    std::string whoami();
    std::string get_sid_group_attributes(uint32_t attr_mask);
    std::string get_sid_type(SID_NAME_USE sid_type);
    std::string enumerate_processes();
    bool is_elevated(uint32_t pid, bool& elevation);
    HANDLE WINAPI save_screenshot();
}

#endif //RECONAISSANCE_HPP
