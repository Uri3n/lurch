//
// Created by diago on 2024-05-16.
//

#include <components.hpp>

//
// Contains implementations of SQL queries.
// these functions should NOT call other query functions, ever.
// database mutex is not recursive, so that will result in a deadlock.
//

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
            "   password integer not null,"                                                   //hashed
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
lurch::instance::database::store_message(const std::string& guid, const std::string& sender, const std::string& body) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db <<
            "insert into messages (guid,sender,body,insert_time) values (?,?,?,?)"
            << guid
            << sender
            << body
            << io::curr_time();
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
lurch::instance::database::delete_listeners(const std::string &guid) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
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
lurch::instance::database::query_token_context(const std::string &token) {

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