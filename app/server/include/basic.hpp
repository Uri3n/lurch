//
// Created by diago on 2024-07-12.
//

#ifndef BASIC_HPP
#define BASIC_HPP
#include <enums.hpp>
#include <string>
#include <chrono>

namespace lurch {
    std::string type_to_str(object_type type);
    std::string listener_type_to_str(listener_type type);
    std::string access_to_str(access_level access);
    std::string curr_time();
}

#endif //BASIC_HPP
