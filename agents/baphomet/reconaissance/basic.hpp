//
// Created by diago on 2024-05-25.
//

#ifndef BASIC_HPP
#define BASIC_HPP
#include <Windows.h>
#include "../util/common.hpp"
#include "../util/function_ptrs.hpp"
#include "../util/io.hpp"
#include "../util/structs.hpp"

#define LURCH_INVALID_INTEGRITY_LEVEL 0xFFFFUL
#define WHOAMI_SID_DIVIDING_CHARS "============================================================== " \
                                  "================= "                                              \
                                  "============= "                                                  \
                                  "===================================================="

#define WHOAMI_PRIVS_DIVIDING_CHARS "============================= " \
                                    "======== "

namespace recon {
    bool        get_desktop_name(std::string& desktop_name);//-
    bool        get_host_name(std::string& host_name); //-
    bool        get_logged_on_user(std::string& user); //-
    bool        get_current_process_name(std::string& process_name); //-
    bool        get_user_sid(std::string& sid); //-
    bool        get_integrity_level(std::string& integrity_lvl_str); //-
    uint32_t    get_current_process_pid(); //--
    uint32_t    get_major_os_version(); //--
    std::string whoami();
}

#endif //BASIC_HPP
