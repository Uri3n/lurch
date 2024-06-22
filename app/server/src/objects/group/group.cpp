//
// Created by diago on 2024-05-01.
//

#include <group.hpp>
#include <components.hpp>

lurch::result<std::filesystem::path>
lurch::group::upload(const std::string &file, const std::string &extension) {
    return error("dfdas");
}

lurch::result<std::string>
lurch::group::receive(reciever_context& ctx) {
    return OBJECT_EMPTY_RESPONSE;
}
