//
// Created by diago on 2024-06-24.
//

#include <components.hpp>



lurch::result<lurch::access_level>
lurch::instance::database::match_user(const std::string& username, const std::string& password) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        for(auto&& row : *this->db << "select username,password,access_level from users where username = ?" << username) {
            std::string q_username;
            uint32_t q_password = 0;
            int32_t q_access;

            row >> q_username >> q_password >> q_access;
            if(q_username == username && q_password == hash_password(password)) {
                return static_cast<access_level>(q_access);
            }
        }

        throw std::runtime_error("user does not exist.");
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}


bool
lurch::instance::database::match_token(const std::string &token, const access_level required_access) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        for(auto&& row : *this->db << u"select token,access_level from tokens where token = ? and expiration_time > strftime('%s', 'now')" << token) {
            std::string q_token;
            int32_t q_access;

            row >> q_token >> q_access;
            if(required_access > static_cast<access_level>(q_access)) {
                throw std::runtime_error("bad token access level");
            }

            return true;
        }

        throw std::runtime_error("token does not exist.");
    }
    catch(const std::exception& e) {
        inst->log.write(std::string("match_token: ") + e.what(), log_type::ERROR_MINOR, log_noise::REGULAR);
        return false;
    }
}


lurch::result<lurch::object_type>
lurch::instance::database::query_object_type(const std::string &guid) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        for(auto&& row : *this->db << "select type from objects where guid = ?" << guid) {
            int64_t q_object_type = 0;
            row >> q_object_type;
            return static_cast<object_type>(q_object_type);
        }

        throw std::runtime_error("object does not exist.");
    }
    catch (const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("unknown exception encountered");
    }
}


lurch::result<lurch::array_of_children>
lurch::instance::database::query_object_children(const std::string &guid){

    array_of_children children;
    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        for(auto&& row : *this->db << "select guid,alias,type,type_index from objects where parent = ?" << guid) {

            std::string q_guid;
            std::string q_alias;
            int64_t     q_type = 0;
            int64_t     q_index = 0;

            row >> q_guid >> q_alias >> q_type >> q_index;
            children.emplace_back(std::make_tuple(
                q_guid,
                q_alias,
                static_cast<object_type>(q_type),
                static_cast<object_index>(q_index)
            ));
        }

        if(children.empty()) {
            throw std::runtime_error("object has no children.");
        }

    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("unknown exception encountered");
    }

    return children;
}


lurch::result<lurch::object_data>
lurch::instance::database::query_object_data(const std::string &guid) {

    if(guid.empty()) {
        return error("empty GUID provided.");
    }

    std::optional<object_data> data = std::nullopt;
    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        *this->db << "select parent,alias,type,type_index from objects where guid = ? ;"
            << guid
            >> [&](std::unique_ptr<std::string> parent, std::string alias, int64_t type, int64_t index) {
                data = std::make_tuple(
                    (parent == nullptr ? std::string("none") : *parent),
                    alias,
                    static_cast<object_type>(type),
                    static_cast<object_index>(index)
                );
            };

        if(data) {
            return *data;
        }

        throw std::runtime_error("object does not exist.");

    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("unknown exception encountered");
    }
}


lurch::result<std::vector<lurch::object_message>>
lurch::instance::database::query_object_messages(const std::string &guid, const int message_index) {

    std::vector<object_message> messages;
    std::lock_guard<std::mutex> lock(this->mtx);

    try {

        int _index = 0;
        for(auto&& row : *this->db << "select sender,body,insert_time from messages where guid = ? order by _id desc;" << guid) {
            if(_index >= message_index) {
                std::string q_sender;
                std::string q_body;
                std::string q_time;

                row >> q_sender >> q_body >> q_time;
                messages.emplace_back(std::make_tuple(q_sender, q_body, q_time));

                if(_index - message_index >= 20) {
                    break;
                }
            }

            ++_index;
        }

        if(messages.empty()) {
            throw std::runtime_error("no messages found.");
        }
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("unknown exception encountered");
    }

    return messages;
}


lurch::result<std::vector<lurch::full_token_data>>
lurch::instance::database::query_full_token_list() {

    std::lock_guard<std::mutex> lock(this->mtx);
    std::vector<full_token_data> tokens;

    try {
        for(auto&& row : *this->db << "select token,datetime(expiration_time, 'unixepoch'),alias,access_level from tokens") {
            std::string q_token;
            std::string q_datetime;
            std::string q_alias;
            int32_t q_access;

            row >> q_token >> q_datetime >> q_alias >> q_access;
            tokens.emplace_back(full_token_data {
                .token = q_token,
                .expiration = q_datetime,
                .alias = q_alias,
                .access = static_cast<access_level>(q_access),
            });
        }

        if(tokens.empty()) {
            throw std::runtime_error("no tokens exist.");
        }

        return tokens;
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}


lurch::result<std::vector<lurch::listener_data>>
lurch::instance::database::query_all_listeners() {

    std::lock_guard<std::mutex> lock(this->mtx);
    std::vector<listener_data>  listeners;

    try {
        for(auto&& row : *this->db << "select address, port, object_guid, type, certificate, key from listeners;") {

            std::string                  q_address;
            int64_t                      q_port;
            std::string                  q_guid;
            int64_t                      q_type;
            std::unique_ptr<std::string> q_certificate  = nullptr;
            std::unique_ptr<std::string> q_key          = nullptr;


            row >> q_address >> q_port >> q_guid >> q_type >> q_certificate >> q_key;

            listeners.emplace_back(
                q_address,
                q_guid,
                q_port,
                static_cast<listener_type>(q_type),
                q_certificate == nullptr ? std::nullopt : std::optional(*q_certificate),
                q_key == nullptr ? std::nullopt : std::optional(*q_key)
            );
        }

        if(listeners.empty()) {
            throw std::runtime_error("No listeners exist.");
        }

        return listeners;
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("Unknown error.");
    }
}


lurch::result<std::vector<lurch::listener_data>>
lurch::instance::database::query_listeners_by_object(const std::string &guid) {

    std::lock_guard<std::mutex> lock(this->mtx);
    std::vector<listener_data>  listeners;

    try {
        for(auto&& row : *this->db << "select address, port, object_guid, type, certificate, key from listeners where object_guid = ?;" << guid) {

            std::string                  q_address;
            int64_t                      q_port;
            std::string                  q_guid;
            int64_t                      q_type;
            std::unique_ptr<std::string> q_certificate  = nullptr;
            std::unique_ptr<std::string> q_key          = nullptr;


            row >> q_address >> q_port >> q_guid >> q_type >> q_certificate >> q_key;

            listeners.emplace_back(
                q_address,
                q_guid,
                q_port,
                static_cast<listener_type>(q_type),
                q_certificate == nullptr ? std::nullopt :  std::optional(*q_certificate),
                q_key == nullptr ? std::nullopt : std::optional(*q_key)
            );
        }

        if(listeners.empty()) {
            throw std::runtime_error("No listeners currently exist for this object.");
        }

        return listeners;
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("Unknown error.");
    }
}


size_t
lurch::instance::database::object_count() {
    std::lock_guard<std::mutex> lock(this->mtx);

    // this shit sucks
    try {
        size_t count = 0;
        *this->db << "select count(*) from objects" >> count;
        return count;
    }
    catch(...) {
        return 0;
    }
}


lurch::result<std::string>
lurch::instance::database::query_root_guid() {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        for(auto&& row : *this->db << "select guid from objects where parent is null") {
            std::string q_guid;
            row >> q_guid;
            return q_guid;
        }

        throw std::runtime_error("root GUID does not exist.");

    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}


lurch::result<std::pair<std::string, lurch::access_level>>
lurch::instance::database::query_token_context(const std::string& token) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        for(auto&& row : *this->db << u"select alias,access_level from tokens where token = ?" << token) {
            std::string q_alias;
            int32_t q_access;

            row >> q_alias >> q_access;
            return std::make_pair(q_alias, static_cast<access_level>(q_access));
        }

        throw std::runtime_error("token does not exist.");
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}