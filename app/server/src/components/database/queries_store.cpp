//
// Created by diago on 2024-06-24.
//

#include <components.hpp>

lurch::result<bool>
lurch::instance::database::initialize(
    instance* inst,
    const std::optional<std::string>& initial_user,
    const std::optional<std::string>& initial_password
) {

    try {
        this->inst = inst;
        this->db   = std::make_unique<sqlite::database>("db/lurch_database.db");

        *this->db <<
            "create table if not exists users ("
            "   _id integer primary key autoincrement not null,"
            "   username text unique not null,"
            "   password integer not null," //hashed
            "   access_level integer not null"
            ");";

        *this->db <<
            "create table if not exists tokens ("
            "   token text unique not null,"
            "   expiration_time integer not null,"
            "   alias text not null,"
            "   access_level integer not null"
            ");";

        *this->db <<
            "create table if not exists objects ("
            "   _id integer primary key autoincrement not null,"
            "   guid text unique not null,"
            "   parent text," //can be null.
            "   alias text not null,"
            "   type integer not null,"
            "   type_index integer not null"
            ");";

        *this->db <<
            "create table if not exists messages ("
            "   _id integer primary key autoincrement not null,"
            "   guid text not null,"
            "   sender text not null,"
            "   body text not null,"
            "   insert_time text not null"
            ");";

        *this->db <<
            "create table if not exists listeners ("
            "   _id integer primary key autoincrement not null,"
            "   address text not null,"
            "   port integer not null,"
            "   object_guid text not null,"
            "   type integer not null,"
            "   certificate text,"
            "   key text"
            ");";


        if(initial_user && initial_password) {
            *this->db <<
                "insert into users (username,password,access_level) values (?,?,?);"
                << initial_user.value()
                << hash_password(initial_password.value())
                << static_cast<int32_t>(access_level::HIGH);
        }
    }
    catch(const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("unknown exception encountered");
    }

    //
    // let's do this real quick
    //

    if(!std::filesystem::exists("static/fileman/")) {
        std::filesystem::create_directory("static/fileman/");
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::store_user(const std::string &username, const std::string &password, const access_level access) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db <<
            "insert into users (username,password,access_level) values (?,?,?);"
            << username
            << hash_password(password)
            << static_cast<int32_t>(access);

    }
    catch(const std::exception& e) {
        return error(io::format_str("exception: {}", e.what()));
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::store_message(const std::string& guid, const std::string& sender, const std::string& body) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db <<
            "insert into messages (guid,sender,body,insert_time) values (?,?,?,?)"
            << guid
            << sender
            << body
            << curr_time();
    }
    catch(const std::exception& e) {
        return error(io::format_str("exception: {}", e.what()));
    }
    catch(...) {
        return error("unknown exception encountered");
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::store_token(
        const std::string &token,
        const access_level access,
        const std::optional<std::string>& alias,
        uint64_t expiration_time
    ) {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        *this->db << u"insert into tokens (token,expiration_time,alias,access_level) values (?,strftime('%s', 'now') + ?,?,?);"
            << token
            << expiration_time * 3600
            << alias.value_or("unknown")
            << static_cast<int32_t>(access);
    }
    catch(const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    }
    catch(...) {
        return error("unknown exception.");
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::store_listener(
        const std::string                 &address,
        const int64_t                     port,
        const std::string                 &object_guid,
        const std::optional<std::string>  certificate_path, // Only used if HTTPS
        const std::optional<std::string>  key_path,         // Only used if HTTPS
        const listener_type               type
    ) {

    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        if(certificate_path && key_path) {
            *this->db << "insert into listeners (address, port, object_guid, type, certificate, key) values (?,?,?,?,?,?);"
                << address
                << port
                << object_guid
                << static_cast<int64_t>(type)
                << *certificate_path
                << *key_path;
        }
        else {
            *this->db << "insert into listeners (address, port, object_guid, type) values (?,?,?,?);"
                << address
                << port
                << object_guid
                << static_cast<int64_t>(type);
        }
    }
    catch(const std::exception& e) {
        return error(e.what());
    }

    return { true };
}


lurch::result<bool>
lurch::instance::database::store_object(
        const std::string &guid,
        const std::optional<std::string> parent,
        const std::string alias,
        const object_type type,
        const object_index index
 ) {

    if(alias.empty() || guid.empty()) {
        return error("Invalid parameters.");
    }

    std::lock_guard<std::mutex> lock(this->mtx);
    try {

        // Note: this is really fucking shit.
        // However, I cannot simply do (parent.has_value() ? parent.value() : nullptr).
        // The compiler does not like this (assumedly because the two ternary options are unrelated types?)
        // anyways, I'll be sticking to this for now.
        if(parent.has_value()) {
            *this->db <<
                "insert into objects (guid,alias,parent,type,type_index) values (?,?,?,?,?);"
                << guid
                << alias
                << parent.value()
                << static_cast<int64_t>(type)
                << static_cast<int64_t>(index);
        } else {
            *this->db <<
                "insert into objects (guid,alias,parent,type,type_index) values (?,?,?,?,?);"
                << guid
                << alias
                << nullptr
                << static_cast<int64_t>(type)
                << static_cast<int64_t>(index);
        }

    }
    catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    }
    catch(...) {
        return error("unknown exception encountered");
    }

    return { true };
}