//
// Created by diago on 2024-05-25.
//

#ifndef SCREENSHOT_HPP
#define SCREENSHOT_HPP
#include <Windows.h>
#include <string>
#include "../util/common.hpp"
#include "../util/io.hpp"

namespace recon {
    HANDLE WINAPI save_screenshot();
}

#endif //SCREENSHOT_HPP
