//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

bool lurch::instance::database::match_user(const std::string& username, const std::string& password) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        for(auto&& row : *this->db << "select username,password from users where username = ?" << username) {
            std::string q_username; uint32_t q_password = 0;

            row >> q_username >> q_password;
            if(q_username == username && q_password == hash_password(password)) {
                return true;
            }
        }

        throw std::exception();
    } catch(...) {
        io::failure(io::format_str("Query for user: {}, password: {} failed.", username, password));
        return false;
    }
}

uint32_t lurch::instance::database::hash_password(const std::string &password) {

    static constexpr uint32_t initial_seed = 7; //can be altered if needed
    uint32_t hash = 0;
    size_t index = 0;

    while(index != password.size()) {
        hash += password[index++];
        hash += hash << initial_seed;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}


lurch::result<bool> lurch::instance::database::initialize(
    lurch::instance* inst,
    const std::optional<std::string >& initial_user,
    const std::optional<std::string>& initial_password
    ) {

    try {
        this->inst = inst;
        this->db = std::make_unique<sqlite::database>("db/lurch_database.db");

        *this->db <<
            "create table if not exists users ("
            "   _id integer primary key autoincrement not null,"
            "   username text unique,"
            "   password integer" //hashed
            ");";

        if(initial_user && initial_password) {
            *this->db <<
                "insert into users (username,password) values (?,?);"
                << initial_user.value()
                << hash_password(initial_password.value());
        }

    } catch(const sqlite::sqlite_exception& e) {
        return lurch::error(io::format_str("exception: {}\nSQL: {}", e.what(), e.get_sql()));
    } catch(...) {
        return lurch::error("unknown exception encountered");
    }

    return lurch::result<bool>(true);
}