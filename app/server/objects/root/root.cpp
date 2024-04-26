//
// Created by diago on 2024-04-25.
//

#include "root.hpp"

std::string lurch::root::recieve(const lurch::command &cmd) {
    return std::string("got this: ") + cmd.name;
}
