//
// Created by diago on 2024-06-24.
//

#include <group.hpp>
#include <components.hpp>

lurch::result<std::filesystem::path>
lurch::group::upload(const std::string& file, const std::string& extension) {
    return error("This object does not accept files.");
}

lurch::result<std::string>
lurch::group::receive(reciever_context& ctx) {

    if(!commands.ready()) {
        init_commands();
    }

    if(!commands.matches(ctx.cmd)) {
        return error("invalid command or argument");
    }

    if(callables.contains(ctx.cmd.name)) {
        return callables[ctx.cmd.name](this, ctx);
    }

    return OBJECT_EMPTY_RESPONSE;
}

