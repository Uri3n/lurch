//
// Created by diago on 2024-04-25.
//

#include "agent.hpp"

std::string
lurch::agent::recieve(const command &cmd) {
    return std::string("got ya shit: ") + cmd.name;
}
