//
// Created by diago on 2024-06-06.
//

#include "root.hpp"
#include "../../components/instance.hpp"

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