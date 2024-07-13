//
// Created by diago on 2024-06-05.
//

#ifndef STRUCTS_HPP
#define STRUCTS_HPP
#include <macro.hpp>
#include <enums.hpp>
#include <expected>
#include <ostream>
#include <string>
#include <optional>
#include <command_types.hpp>

namespace lurch {

    struct listener_data {
        std::string                 address;
        std::string                 object_guid;
        int64_t                     port;
        listener_type               type;
        std::optional<std::string>  certificate_path;
        std::optional<std::string>  key_path;
    };

    // @Cleanup: Why are there two of these anyway?
    struct token_context {
        std::string         token;
        std::string         alias;
        access_level        access = access_level::LOW;
    };

    struct full_token_data {
        std::string         token;
        std::string         expiration;
        std::string         alias;
        access_level        access = access_level::LOW;
    };

    struct config_data {
        std::string         bindaddr;
        uint16_t            port;
        bool                use_https;
        std::string         cert_path;
        std::string         key_path;
    };

    struct flag_descriptor {
        std::string full_name;
        std::string type_str;
        std::string required;
        std::string description;
    };

    struct search_ctx {
        result<std::string> response;
        access_level        obj_access      = access_level::LOW;
        bool                keep_going      = true;
        bool                log_if_error    = true;
    };

    struct baphomet_metadata {
        std::string     addr;
        std::string     user_agent;
        std::string     callback_object;
        std::string     token;
        int64_t         port;
        uint64_t        sleep_time;
        uint64_t        jitter;
        bool            use_sleepmask;
        bool            prevent_debugging;
        listener_type   proto;
    };

    struct reciever_context {
        token_context   tok;
        command         cmd;
        std::string     message_raw;
        std::string     address;
        bool            log_if_error   = true;
        bool            delete_self    = false;
    };
}

#endif //STRUCTS_HPP
