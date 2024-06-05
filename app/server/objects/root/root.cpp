//
// Created by diago on 2024-04-25.
//

#include "root.hpp"
#include "../../components/instance.hpp"

namespace lurch {
    accepted_commands root::commands;
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

        commands.done();
    }
}


void
lurch::root::shutdown(const bool wipe_files) const {

    if(wipe_files) {
        inst->db.fileman_wipe_all();
    }

    inst->log.write("Server is shutting down...", log_type::INFO, log_noise::NOISY);
    inst->set_shutdown_condition();
}


lurch::result<std::string>
lurch::root::generate_token(const command &cmd) const {

    const auto [alias, access, expiration] =
        cmd.get<std::string>("--alias", "-a")
            .with<int64_t>("--access-level", "-al")
            .with<int64_t>("--expiration-hours", "-e")
            .done();

    if(*access > 2 || *access < 0) {
        return "invalid access level provided. Value must be 0 - 2.";
    }

    if(expiration.has_value() && (*expiration > 8765 || *expiration < 0)) {
        return "expiration in hours must be less than one year, and greater than zero.";
    }

    const std::string new_token = inst->db.generate_token();
    return inst->db.store_token(new_token, static_cast<access_level>(*access), *alias, expiration.value_or(12))
        .and_then([&](const bool _) {
            return result<std::string>(
                io::format_str("Successfully created token {} with access level {}.",
                    new_token,
                    io::access_to_str(static_cast<access_level>(*access))
                ));
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
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

    const std::string guid = *std::get<0>(cmd.get<std::string>("--guid", "-g").done());
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

    return inst->db.store_user(*username, *password, (*grant_admin ? access_level::HIGH : access_level::MEDIUM))
        .and_then([&](const bool _) {
            inst->log.write(io::format_str("created new user:\n {}", *username), log_type::SUCCESS, log_noise::NOISY);
            return result<std::string>("success.");
        })
        .or_else([&](std::string err) {
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

        for(const auto&[token, expiration, alias, access] : *tokens_res) {
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
lurch::root::recieve(const command &cmd, bool& log_if_error) {

    if(!commands.ready()) {
        init_commands();
    }

    if(!commands.matches(cmd)) {
        return error("invalid command or argument.");
    }

    if(cmd == "shutdown") {
        shutdown(std::get<0>(cmd.get<empty>("--wipe-files", "-wf").done()).has_value());
        return "shutting down server...";
    }

    if(cmd == "add_user") {
        return add_user(cmd);
    }

    if(cmd == "remove_user") {
        return remove_user(*std::get<0>(cmd.get<std::string>("--username", "-u").done()));
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

    if(cmd == "generate_token") {
        return generate_token(cmd);
    }


    return OBJECT_EMPTY_RESPONSE;
}
