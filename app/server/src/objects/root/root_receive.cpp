//
// Created by diago on 2024-06-06.
//

#include <root.hpp>

lurch::result<std::filesystem::path>
lurch::root::upload(const std::string &file, const std::string &extension) {
    return error("this object does not accept files.");
}


lurch::result<std::string>
lurch::root::receive(reciever_context& ctx) {

    if(!commands.ready()) {
        init_commands();
    }

    if(!commands.matches(ctx.cmd)) {
        return error("invalid command or argument.");
    }

    if(callables.contains(ctx.cmd.name)) {
        return callables[ctx.cmd.name](this, ctx);
    }

    return OBJECT_EMPTY_RESPONSE;
}