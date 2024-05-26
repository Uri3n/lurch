//
// Created by diago on 2024-05-25.
//

#ifndef PROCENUM_HPP
#define PROCENUM_HPP
#include <Windows.h>
#include <winternl.h>
#include <string>
#include "../util/common.hpp"
#include "../util/function_ptrs.hpp"
#include "../util/io.hpp"

namespace recon {
    std::string enumerate_processes();
    bool is_elevated(uint32_t pid, bool& elevation);
}

#endif //PROCENUM_HPP
