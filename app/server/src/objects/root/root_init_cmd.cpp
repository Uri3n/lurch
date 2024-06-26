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
            .arg<empty>("--wipe-files", "-wf", false)
            .desc("If this flag is specified, all staged, exfiltrated, and stored files are wiped.")

            .arg<empty>("--wipe-messages", "-wm", false)
            .desc("If this flag is specified, all sent messages are wiped from the database.")

            .arg<empty>("--wipe-tokens", "-wt", false)
            .desc("If this flag is specified, all access tokens are wiped from the database.");


        commands.add_command("add_user", "adds a new user to the database.")
            .arg<std::string>("--username", "-u", true)
            .desc("The name of the user to be added. This should be unique.")

            .arg<std::string>("--password", "-p", true)
            .desc("The password for the new user.")

            .arg<bool>("--grant-admin", "-a", true)
            .desc("If this is true, the user has admin privileges.<br>"
                  "This means they can access the root object, remove users, and manage configurations.");


        commands.add_command("remove_user", "removes a user from the database.")
            .arg<std::string>("--username", "-u", true)
            .desc("the name of the user to be removed.");


        commands.add_command("remove_child", "deletes a specified child given it's GUID.")
            .arg<std::string>("--guid", "-g", true);


        commands.add_command("generate_token", "generates an arbitrary access token with a specified expiration time.")
            .arg<std::string>("--alias", "-a", true)
            .desc("the alias to be given to the token. This does not have to be unique.")

            .arg<int64_t>("--access-level", "-al", true)
            .desc("The access level of the token. Can be LOW (0), MEDIUM (1) or HIGH (2).")

            .arg<int64_t>("--expiration-hours", "-e", false)
            .desc("The time, in hours, after which the token should be destroyed. 12 by default.");


        commands.add_command("help", "display this help message.")
            .arg<std::string>("--command", "-c", false)
            .desc("If specified, shows information about a specific command.");

        commands.add_command("delete_token", "deletes a specified access token")
            .arg<std::string>("--token", "-t", true)
            .desc("The value specified by this flag should be a token to be deleted.");

        commands.add_command("create", "creates a child under the root object.")
            .arg<std::string>("--object", "-o", true)
            .desc("Specifies the type of object to create. Currently supported values are:<br>"
                  " - \"baphomet\"<br>"
                  " - \"generic group\"<br>");


        commands.add_command("tokens", "displays existing session tokens and their context.");
        commands.add_command("listeners", "displays all active listeners on the server.");
        commands.done();


        callables =
        {
            {"shutdown",         &root::shutdown},
            {"add_user",         &root::add_user},
            {"remove_child",     &root::remove_child},
            {"remove_user",      &root::remove_user},
            {"delete_token",     &root::delete_token},
            {"generate_token",   &root::generate_token},
            {"create",           &root::create},
            {"tokens",           [](root* ptr, reciever_context& ctx) { return ptr->get_tokens(); }},
            {"listeners",        [](root* ptr, reciever_context& ctx){ return ptr->get_listeners(); }},
            {"help",             [&](root* ptr, reciever_context& ctx) {

              const auto [command] = ctx.cmd.get<std::string>("--command","-c").done();
              if(command) {
                  return commands.command_help(*command);
              }

              return commands.help();
            }},
        };
    }
}