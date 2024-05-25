//
// Created by diago on 2024-04-25.
//

#include "root.hpp"
#include "../../components/instance.hpp"

void
lurch::root::shutdown(const bool wipe_files) const {

    if(wipe_files) {
        inst->db.fileman_wipe_all();
    }

    {
        std::lock_guard<std::mutex> lock (inst->mtx);
        inst->shutdown = true;
        inst->shutdown_condition.notify_all();
    }
}


lurch::result<std::string>
lurch::root::create_chatroom(const command &cmd) {

    return create_child(object_index::GENERIC_CHATROOM, object_type::EXTERNAL, "Teamserver Chat")
        .and_then([&](const bool _) {
            return result<std::string>("successfully created chatroom object.");
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::root::remove_child(const command &cmd) {

    const std::string guid = std::get<0>(cmd.get<std::string>("--guid", "-g").done()).value();
    return delete_child(guid)
        .and_then([&](const bool _) {
            return result<std::string>("successfully deleted child.");
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::root::add_user(const command& cmd) const {

    const auto &[username, password, grant_admin] =
        cmd.get<std::string>("--username", "-u")
            .with<std::string>("--password", "-p")
            .with<bool>("--grant-admin", "-a")
            .done();

    return inst->db.store_user(username.value(), password.value(), (grant_admin.value() ? access_level::HIGH : access_level::MEDIUM))
        .and_then([&](const bool _) {
            inst->routing.send_ws_notification(io::format_str("created new user:\n {}", username.value()), ws_notification_intent::GOOD);
            return result<std::string>("success.");
        })
        .or_else([&](std::string err) {
           inst->routing.send_ws_notification(io::format_str("failed to create new user:\n{}", err), ws_notification_intent::BAD);
           return result<std::string>(error(err));
        });
}


lurch::result<std::string> lurch::root::remove_user(const std::string &user) const {
    return inst->db.delete_user(user)
        .and_then([&](const bool _) {
            return result<std::string>("success");
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::root::get_tokens() const {

    if(const auto tokens_res = inst->db.query_full_token_list()) {

        std::string buff;
        buff += io::format_str("{:<40} {:<23} {:<23} {:<6}", "Token", "Expiration", "Alias", "Access Level") + '\n';
        buff += io::format_str("{:=<40} {:=<23} {:=<23} {:=<6}", "=", "=", "=", "=") + '\n';

        for(const auto&[token, expiration, alias, access] : tokens_res.value()) {
            buff += io::format_str(
                "{:<40} {:<23} {:<23} {:<6}",
                token,
                expiration,
                alias,
                io::access_to_str(access)
            ) + '\n';
        }

        return { buff };
    }
    else {
        return error(tokens_res.error());
    }
}


lurch::result<std::filesystem::path>
lurch::root::upload(const std::string &file, const std::string &extension) {
    return error("this object does not accept files.");
}


lurch::result<std::string>
lurch::root::recieve(const command &cmd) {

    static accepted_commands commands;

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

        commands.add_command("help", "display this help message.");
        commands.add_command("create_chatroom", "creates a new chatroom object as a child.");
        commands.add_command("tokens", "displays existing session tokens and their context.");

        commands.done();
    }

    if(!commands.matches(cmd)) {
        return error("invalid command or argument.");
    }


    if(cmd == "shutdown") {
        inst->routing.send_ws_notification("Server is shutting down...", ws_notification_intent::NEUTRAL);
        shutdown(std::get<0>(cmd.get<empty>("--wipe-files", "-wf").done()).has_value());
        return "shutting down server...";
    }

    if(cmd == "add_user") {
        return add_user(cmd);
    }

    if(cmd == "remove_user") {
        return remove_user(std::get<0>(cmd.get<std::string>("--username", "-u").done()).value());
    }

    if(cmd == "remove_child") {
        return remove_child(cmd);
    }

    if(cmd == "create_chatroom") {
        return create_chatroom(cmd);
    }

    if(cmd == "tokens") {
        return get_tokens();
    }

    if(cmd == "help") {
        return { commands.help() };
    }


    return OBJECT_EMPTY_RESPONSE;
}
