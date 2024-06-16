//
// Created by diago on 2024-06-15.
//

#ifndef ANTI_ANALYSIS_HPP
#define ANTI_ANALYSIS_HPP
#include <Windows.h>
#include <string>
#include <macro.hpp>
#include <common.hpp>
#include <function_ptrs.hpp>
#include <structs.hpp>
#include <misc.hpp>

namespace anti_analysis {

    bool delete_self();

    bool debug_check_1();
    bool debug_check_2();
    bool debug_check_3();
    bool debug_check_4();
    bool debug_check_5();
    bool being_debugged();
}

#endif //ANTI_ANALYSIS_HPP
