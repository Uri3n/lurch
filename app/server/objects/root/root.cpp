//
// Created by diago on 2024-04-25.
//

#include "root.hpp"
#include "../../components/instance.hpp"

lurch::result<std::filesystem::path>
lurch::root::upload(const std::string &file, const std::string &extension) {
    return error("dfdsf");
}

lurch::result<std::string>
lurch::root::recieve(const command &cmd) {

    static accepted_commands commands;

    if(!commands.ready()) {
        commands.add_command("shutdown")
            .arg<empty>("--wipe-files", "-wf", false);

        commands.add_command("add_user")
            .arg<std::string>("--username", "-u", true)
            .arg<std::string>("--password", "-p", true)
            .arg<bool>("--grant-admin", "-a", true);

        commands.add_command("remove_user")
            .arg<std::string>("--username", "-u", true);

        commands.add_command("create_chatroom")
            .arg<std::string>("--type", "-u", false);

        commands.add_command("help")
            .arg<std::string>("--command", "-c", false);

        commands.done();
    }

    if(!commands.matches(cmd)) {
        return error("invalid command or argument.");
    }

    if(cmd == "shutdown") {
        {
            std::lock_guard<std::mutex> lock(inst->mtx);
            inst->shutdown = true;
            inst->shutdown_condition.notify_all();
        }

        return { "yay" };
    }


    return error("fdsfsdfds");
}
