//
// Created by diago on 2024-05-18.
//

#include <chatroom.hpp>
#include <components.hpp>

//
// The chatroom object is very simple. It has no commands, and accepts
// all file types. When we receive a message, we can just send back an empty response.
// When we receive a file, we just need to create the file and send back the path to it.
//

lurch::result<std::filesystem::path>
lurch::chatroom::upload(const std::string &file, const std::string &extension) {
    return inst->db.fileman_create(file, extension, id, true);
}

lurch::result<std::string>
lurch::chatroom::receive(reciever_context& ctx) {
    return OBJECT_EMPTY_RESPONSE;
}
