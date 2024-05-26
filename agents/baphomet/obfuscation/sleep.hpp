//
// Created by diago on 2024-05-24.
//

#ifndef SLEEP_HPP
#define SLEEP_HPP
#include <Windows.h>
#include <cstdint>
#include <ctime>
#include "../util/structs.hpp"
#include "../util/common.hpp"

namespace obfus {
    void sleep(uint32_t sleep_time);
}

void init_rc4_key(USTRING* pKey);
uint8_t* get_implant_base_address();

#endif //SLEEP_HPP
