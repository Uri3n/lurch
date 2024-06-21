//
// Created by diago on 2024-06-06.
//

#include <root.hpp>

namespace lurch {
    accepted_commands root::commands;
    std::unordered_map<std::string, std::function<result<std::string>(root*, reciever_context&)>> root::callables;
}

void
lurch::root::init_commands() {

    if(!commands.ready()) {
        commands.add_command("shutdown", "shuts down the teamserver.")
            .arg<empty>("--wipe-files", "-wf", false);

        commands.add_command("add_user", "adds a new user to the database.")
            .arg<std::string>("--username", "-u", true)
            .arg<std::string>("--password", "-p", true)
            .arg<bool>("--grant-admin", "-a", true);

        commands.add_command("remove_user", "removes a user from the database.")
            .arg<std::string>("--username", "-u", true);

        commands.add_command("remove_child", "deletes a specified child given it's GUID.")
            .arg<std::string>("--guid", "-g", true);

        commands.add_command("generate_token", "generates an arbitrary access token with a specified expiration time.")
            .arg<std::string>("--alias", "-a", true)
            .arg<int64_t>("--access-level", "-al", true)
            .arg<int64_t>("--expiration-hours", "-e", false);

        commands.add_command("help", "display this help message.");
        commands.add_command("create_chatroom", "creates a new chatroom object as a child.");
        commands.add_command("tokens", "displays existing session tokens and their context.");
        commands.add_command("listeners", "displays all active listeners on the server.");

        commands.done();

        callables =
        {
            {"shutdown",         &root::shutdown},
            {"add_user",         &root::add_user},
            {"remove_child",     &root::remove_child},
            {"remove_user",      &root::remove_user},
            {"generate_token",   &root::generate_token},
            {"create_chatroom",  &root::create_chatroom},
            {"tokens", [](root* ptr, reciever_context& ctx) { return ptr->get_tokens(); }},
            {"listeners", [](root* ptr, reciever_context& ctx){ return ptr->get_listeners(); }},
            {"help", [&](root* ptr, reciever_context& ctx) { return root::commands.help(); }},
        };
    }
}