//
// Created by diago on 2024-06-24.
//

#include <components.hpp>

lurch::result<bool>
lurch::instance::database::delete_listeners(const std::string &guid) {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        size_t count = 0;
        *this->db << "select count(*) as listener_count from listeners where object_guid = ?" << guid >> count;

        if(!count) {
            throw std::runtime_error("No active listeners.");
        }

        *this->db << "delete from listeners where object_guid = ?;" << guid;
    }
    catch(const std::exception& e) {
        return error(e.what());
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::delete_listener(const std::string &guid, const std::string &address, const int64_t port) {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        size_t count = 0;
        *this->db << "select count(*) as listener_count from listeners where object_guid = ?" << guid >> count;

        if(!count) {
            throw std::runtime_error("No active listeners.");
        }

        *this->db << "delete from listeners where object_guid = ? and address = ? and port = ?;"
            << guid
            << address
            << port;
    }
    catch(const std::exception& e) {
        return error(e.what());
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::delete_object(const std::string &guid) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db << "delete from objects where guid = ?;" << guid;
        *this->db << "delete from messages where guid = ?;" << guid;
    }
    catch (const std::exception& e) {
        return error(io::format_str("exception: {}", e.what()));
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::delete_user(const std::string &username) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        size_t count = 0;
        *this->db << "select count(*) as user_count from users where username = ?" << username >> count;
        if(!count) {
            throw std::runtime_error("user does not exist.");
        }

        *this->db << "delete from users where username = ?;" << username;
    }
    catch(const std::exception& e) {
        return error(e.what());
    }

    return { true };
}


void
lurch::instance::database::delete_old_tokens() {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db << u"delete from tokens where expiration_time < strftime('%s', 'now')";
    }
    catch(...) {
        io::info("no expired tokens.");
    }
}


lurch::result<bool>
lurch::instance::database::delete_all_messages() {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        *this->db << "delete from messages;";
        return { true };
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}


lurch::result<bool>
lurch::instance::database::delete_messages(const std::string& guid) {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        *this->db << "delete from messages where guid = ?" << guid;
        return { true };
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}


lurch::result<bool>
lurch::instance::database::delete_all_tokens() {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        *this->db << "delete from tokens;";
        return { true };
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}


lurch::result<bool>
lurch::instance::database::delete_token(const std::string &token) {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        size_t count = 0;

        *this->db << "select count(*) as token_count from tokens where token = ?;" << token >> count;
        if(!count) {
            throw std::runtime_error("That token doesn't exist.");
        }

        *this->db << "delete from tokens where token = ?;" << token;
        return { true };

    }
    catch(const std::exception& e) {
        return error(e.what());
    }
}
