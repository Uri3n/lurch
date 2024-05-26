//
// Created by diago on 2024-05-25.
//

#ifndef BASIC_HPP
#define BASIC_HPP
#include <Windows.h>
#include "../util/common.hpp"
#include "../util/io.hpp"
#include "../util/structs.hpp"

#define LURCH_INVALID_INTEGRITY_LEVEL 0xFFFFUL

namespace recon {
    bool        get_desktop_name(std::string& desktop_name);
    bool        get_logged_on_user(std::string& user);
    bool        get_current_process_name(std::string& process_name);
    bool        get_user_sid(std::string& sid);
    uint32_t    get_current_process_pid();
    uint32_t    get_major_os_version();
    uint32_t    get_integrity_level();
    std::string whoami();
}

#endif //BASIC_HPP
