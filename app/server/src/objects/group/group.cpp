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
    if(const auto listener_res = inst->routing.start_listener_http("127.0.0.1", 8083, ctx.message_raw, std::nullopt, std::nullopt)) {
        return "Successfully created listener.";
    }
    else {
        return error(listener_res.error());
    }
}
