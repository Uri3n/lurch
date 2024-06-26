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

    //
    // debug purposes only
    //
    if(ctx.cmd.name == "debug_gen") {
        return create_child(object_index::BAPHOMET, object_type::AGENT, "Agent")
            .and_then([&](const bool _) {
                return result<std::string>("successfully created agent object.");
            })
            .or_else([&](std::string err) {
                return result<std::string>(error(err));
            });
    }

    if(!commands.matches(ctx.cmd)) {
        return error("invalid command or argument.");
    }

    if(callables.contains(ctx.cmd.name)) {
        return callables[ctx.cmd.name](this, ctx);
    }

    return OBJECT_EMPTY_RESPONSE;
}