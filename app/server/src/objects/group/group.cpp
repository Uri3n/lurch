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

    if(ctx.cmd == "delete") {
        ctx.delete_self = true;
    }
    if(ctx.cmd == "child") {
        return create_child(object_index::GENERIC_GROUP, object_type::GROUP, "Random Group")
            .and_then([&](const bool _) {
                return result<std::string>("successfully created group object.");
            })
            .or_else([&](std::string err) {
                return result<std::string>(error(err));
            });
    }

    return OBJECT_EMPTY_RESPONSE;
}
