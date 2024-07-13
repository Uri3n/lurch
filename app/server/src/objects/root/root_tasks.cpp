//
// Created by diago on 2024-06-06.
//

#include <root.hpp>
#include <components.hpp>


lurch::result<std::string>
lurch::root::shutdown(reciever_context& ctx) const {

    const auto [wipe_files, wipe_messages, wipe_tokens] =
        ctx.cmd.get<empty>("--wipe-files", "-wf")
            .with<empty>("--wipe-messages", "-wm")
            .with<empty>("--wipe-tokens", "-wt")
            .done();


    if(wipe_files) {
        inst->db.fileman_wipe_all();
    }

    if(wipe_messages) {
        inst->db.delete_all_messages();
    }

    if(wipe_tokens) {
        inst->db.delete_all_tokens();
    }


    inst->log.write("Server is shutting down.", log_type::INFO, log_noise::NOISY);
    inst->set_shutdown_condition();
    return { "Shutting down server." };
}


lurch::result<std::string>
lurch::root::generate_token(reciever_context& ctx) const {

    const auto [alias, access, expiration] =
        ctx.cmd.get<std::string>("--alias", "-a")
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

            const std::string access_as_string = access_to_str(static_cast<access_level>(*access));

            inst->log.write(
                io::format_str("User {} created an access token: {}, access level: {}", ctx.tok.alias, new_token, access_as_string),
                log_type::INFO,
                log_noise::REGULAR
            );

            return result<std::string>(
                io::format_str("Successfully created token {} with access level {}.",
                    new_token,
                    access_as_string
                ));
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::root::create(reciever_context &ctx) {

    const auto [object] = ctx.cmd.get<std::string>("--object", "-o").done();

    object_index index;
    object_type  type;
    std::string  alias;


    if(*object == "baphomet" || *object == "Baphomet") {
        index = object_index::BAPHOMET;
        type  = object_type::AGENT;
        alias = "Baphomet";
    }
    else if(*object == "generic group") {
        index = object_index::GENERIC_GROUP;
        type  = object_type::GROUP;
        alias = "Generic Group";
    }
    else if(*object == "chatroom") {
        index = object_index::GENERIC_CHATROOM;
        type  = object_type::EXTERNAL;
        alias = "TeamServer Chat";
    }
    else {
        return error("Invalid object name.");
    }


    return create_child(index, type, alias)
        .and_then([&](bool _) {
            return result<std::string>("Successfully created child.");
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::root::remove_child(reciever_context& ctx) {

    const auto [guid] = ctx.cmd.get<std::string>("--guid", "-g").done();

    return delete_child(*guid)
        .and_then([&](const bool _) {
            return result<std::string>("successfully deleted child.");
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::root::add_user(reciever_context& ctx) const {

    const auto [username, password, grant_admin] =
        ctx.cmd.get<std::string>("--username", "-u")
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


lurch::result<std::string>
lurch::root::delete_token(reciever_context &ctx) const {

    const auto [token] = ctx.cmd.get<std::string>("--token", "-t").done();

    if(const auto res = inst->db.delete_token(*token)) {

        inst->log.write(
            io::format_str("User {} deleted access token {}", ctx.tok.alias, *token),
            log_type::INFO,
            log_noise::REGULAR
        );

        return "Successfully deleted token " + *token;
    } else {
        return error(res.error());
    }
}





lurch::result<std::string>
lurch::root::remove_user(reciever_context& ctx) const {

    const std::string user = *std::get<0>(ctx.cmd.get<std::string>("--username", "-u").done());

    return inst->db.delete_user(user)
        .and_then([&](const bool _) {

            inst->log.write(
                io::format_str("user {} has been removed by user {}.", user, ctx.tok.alias),
                log_type::INFO,
                log_noise::REGULAR
            );

            return result<std::string>("successfully removed user " + user);
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

        for(const auto &[token, expiration, alias, access] : *tokens_res) {
            buff += io::format_str(
                "{:<40} {:<23} {:<23} {:<6}",
                token,
                expiration,
                alias,
                access_to_str(access)
            ) + '\n';
        }

        return { buff };
    }
    else {
        return error(tokens_res.error());
    }
}


lurch::result<std::string>
lurch::root::get_users() const {

    if(const auto users_res = inst->db.query_all_users()) {
        std::string buff;
        buff += io::format_str("{:<6} {:<40}", "Access", "Username") + '\n';
        buff += io::format_str("{:=<6} {:=<40}", "=", "=") + '\n';

        for(const auto &[username, access] : *users_res) {
            buff += io::format_str("{:<6} {:<40}", access_to_str(access), username) + '\n';
        }

        return { buff };
    }
    else {
        return error(users_res.error());
    }
}


lurch::result<std::string>
lurch::root::get_listeners() const {

    if(const auto listeners = inst->db.query_all_listeners()) {

        std::string buff;
        buff += io::format_str("{:<17} {:<7} {:<7} {:<38}", "Address", "Port", "Proto", "Object") + '\n';
        buff += io::format_str("{:=<17} {:=<7} {:=<7} {:=<38}", "=", "=", "=", "=") + '\n';

        for(const auto &[address, guid, port, type, cert, key] : *listeners) {
            buff += io::format_str(
                "{:<17} {:<7} {:<7} {:<38}",
                address,
                port,
                listener_type_to_str(type),
                guid
            ) + '\n';
        }

        return buff;
    }
    else {
        return error(listeners.error());
    }
}


