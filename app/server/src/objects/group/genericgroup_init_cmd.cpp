//
// Created by diago on 2024-06-24.
//

#include <group.hpp>

namespace lurch {
    accepted_commands group::commands;
    std::unordered_map<std::string, std::function<result<std::string>(group*, reciever_context&)>> group::callables;
}


void
lurch::group::init_commands() {

    if(!commands.ready()) {

        commands.add_command("remove_member", "removes one or all members of the group.")
            .arg<std::string>("--guid", "-g", false)
            .desc("The GUID of the member to remove.")

            .arg<empty>("--all", "-a", false)
            .desc("If this flag is specified, all members are removed. "
                  "Do not use this flag alongside --guid.");


        commands.add_command("add_member", "adds a member to the group")
            .arg<std::string>("--name", "-n", true)
            .desc("This flag should specify a predetermined name that "
                  "represents a particular object. As of right now the only supported value<br>"
                  "for this flag is \"baphomet\".");


        commands.add_command("issue", "sends a specified message to all group members, and returns the output.")
            .arg<std::string>("--message", "-m", true)
            .desc("The message to be sent to the group members.");


        commands.add_command("help", "display this help message or information about a specific command.")
            .arg<std::string>("--command", "-c", false)
            .desc("If specified, shows information about a specific command.");


        commands.add_command("groupfiles", "displays files owned by group members or retrieves a specific one.")
            .arg<std::string>("--get", "-g", false)
            .desc("If this flag is specified, a file that has this name is searched for.<br>"
                  "If found, it is returned as a downloadable link.<br>"
                  "If this flag is not used, all group files are displayed as a list.");


        commands.add_command("members", "displays all members of this group");
        commands.add_command("disband", "deletes the group object and all of its children.");


        commands.done();

        callables =
        {
            {"members",       &group::members},
            {"remove_member", &group::remove_member},
            {"add_member",    &group::add_member},
            {"groupfiles",    &group::groupfiles},
            {"disband",       &group::disband},
            {"issue",         &group::issue},
            {"help",          [&](group* ptr, reciever_context& ctx) {

                const auto [command] = ctx.cmd.get<std::string>("--command","-c").done();
                if(command) {
                    return commands.command_help(*command);
                }

                return commands.help();
            }}
        };

    }
}
