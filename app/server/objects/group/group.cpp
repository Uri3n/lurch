//
// Created by diago on 2024-05-01.
//

#include "group.hpp"

std::string lurch::group::recieve(const lurch::command &cmd) {
    return std::string("got your command: ") + cmd.name;
}
