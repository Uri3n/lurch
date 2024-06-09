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

    if(ctx.cmd == "shutdown") {
        shutdown(std::get<0>(ctx.cmd.get<empty>("--wipe-files", "-wf").done()).has_value());
        return "shutting down server...";
    }

    if(ctx.cmd == "add_user") {
        return add_user(ctx.cmd);
    }

    if(ctx.cmd == "remove_user") {
        return remove_user(*std::get<0>(ctx.cmd.get<std::string>("--username", "-u").done()));
    }

    if(ctx.cmd == "remove_child") {
        return remove_child(ctx.cmd);
    }

    if(ctx.cmd == "create_chatroom") {
        return create_chatroom(ctx.cmd);
    }

    if(ctx.cmd == "tokens") {
        return get_tokens();
    }

    if(ctx.cmd == "help") {
        return { commands.help() };
    }

    if(ctx.cmd == "generate_token") {
        return generate_token(ctx.cmd);
    }


    return OBJECT_EMPTY_RESPONSE;
}