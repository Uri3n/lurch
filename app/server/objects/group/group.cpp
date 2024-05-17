//
// Created by diago on 2024-05-01.
//

#include "group.hpp"

bool lurch::group::upload(const std::string &file, const std::string &extension) {
    return true;
}

std::string lurch::group::recieve(const lurch::command &cmd) {
    return std::string("got your command: ") + cmd.name;
}

std::string lurch::group::download(const std::string &name) {
    return "dfsfsdffff";
}
