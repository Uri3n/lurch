//
// Created by diago on 2024-05-18.
//

#include "chatroom.hpp"
#include "../../components/instance.hpp"

lurch::result<std::filesystem::path>
lurch::chatroom::upload(const std::string &file, const std::string &extension) {
    return inst->db.fileman_create(file, extension, id, true);
}

lurch::result<std::string>
lurch::chatroom::recieve(const command &cmd, bool& log_if_error) {
    return OBJECT_EMPTY_RESPONSE;
}
