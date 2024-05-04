//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

//
// Contains implementations of SQL queries.
// these functions should NOT call other query functions, ever.
// database mutex is not recursive, so that will result in a deadlock.
//

bool
lurch::instance::database::match_user(const std::string& username, const std::string& password) {

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


lurch::result<bool>
lurch::instance::database::store_user(const std::string &username, const std::string &password) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db <<
            "insert into users (username,password) values (?,?);"
            << username
            << hash_password(password);

    } catch(const sqlite::sqlite_exception& e) {
        return lurch::error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(...) {
        return lurch::error("unknown exception encountered");
    }

    return result<bool>(true);
}


uint32_t
lurch::instance::database::hash_password(const std::string &password) {

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


lurch::result<bool>
lurch::instance::database::initialize(
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
            "   insert_time timestamp default current_timestamp"
            ");";

        if(initial_user && initial_password) {
            *this->db <<
                "insert into users (username,password) values (?,?);"
                << initial_user.value()
                << hash_password(initial_password.value());
        }

    } catch(const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(const std::exception& e) {
        return error(e.what());
    } catch(...) {
        return error("unknown exception encountered");
    }

    return lurch::result<bool>(true);
}


lurch::result<bool>
lurch::instance::database::store_message(const std::string& guid, const std::string& sender, const std::string& body) {

    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        *this->db <<
            "insert into messages (guid,sender,body) values (?,?,?)"
            << guid
            << sender
            << body;

    } catch(const sqlite::sqlite_exception& e) {
        io::failure("SQL exception while storing message from: " + sender);
        io::failure(std::string("exception type: ") + e.what());
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(...) {
        return error("unknown exception encountered");
    }

    io::success("stored message. Sender: " + sender);
    return result<bool>(true);
}

lurch::result<bool>
lurch::instance::database::store_object(
        const std::string &GUID,
        const std::optional<std::string> parent,
        const std::string alias,
        const object_type type,
        const object_index index
 ) {

    if(alias.empty() || GUID.empty()) {
        return error("Invalid parameters.");
    }

    std::lock_guard<std::mutex> lock(this->mtx);
    try {

        // Note: this is really fucking shit.
        // However, I cannot simply do (parent.has_value() ? parent.value() : nullptr).
        // The compiler does not like this (assumedly because the two ternary options are unrelated types?)
        // anyways, I'll be sticking to this for now until I can find something better.
        if(parent.has_value()) {
            *this->db <<
                "insert into objects (guid,alias,parent,type,type_index) values (?,?,?,?,?);"
                << GUID
                << alias
                << parent.value()
                << static_cast<int64_t>(type)
                << static_cast<int64_t>(index);
        } else {
            *this->db <<
                "insert into objects (guid,alias,parent,type,type_index) values (?,?,?,?,?);"
                << GUID
                << alias
                << nullptr
                << static_cast<int64_t>(type)
                << static_cast<int64_t>(index);
        }

    } catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(...) {
        return error("unknown exception encountered");
    }

    return result<bool>(true);
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

        //Can be done better... okay for now
        throw std::exception();

    } catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(...) {
        return lurch::error("unknown exception encountered");
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
            int64_t q_type = 0;
            int64_t q_index = 0;

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

    } catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(const std::exception& e) {
        return error(e.what());
    } catch(...) {
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
                        if(parent != nullptr) {
                            data = std::make_tuple(
                                *parent,
                                alias,
                                static_cast<object_type>(type),
                                static_cast<object_index>(index)
                            );
                        }
                    };

        if(data.has_value()) {
            return data.value();
        }

        throw std::runtime_error("object does not exist.");

    } catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(const std::exception& e) {
        return error(e.what());
    } catch(...) {
        return error("unknown exception encountered");
    }
}


lurch::result<std::vector<lurch::object_message>>
lurch::instance::database::query_object_messages(const std::string &guid) {

    std::vector<object_message> messages;
    std::lock_guard<std::mutex> lock(this->mtx);

    try {
        for(auto&& row : *this->db << "select sender,body,insert_time from messages where guid = ? order by _id asc;" << guid) {
            std::string q_sender;
            std::string q_body;
            std::string q_time;

            row >> q_sender >> q_body >> q_time;
            messages.emplace_back(std::make_tuple(q_sender, q_body, q_time));
        }

        if(messages.empty()) {
            throw std::runtime_error("no messages found.");
        }

    } catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(const std::exception& e) {
        return error(e.what());
    } catch(...) {
        return error("unknown exception encountered");
    }

    return messages;
}


size_t
lurch::instance::database::object_count() {
    std::lock_guard<std::mutex> lock(this->mtx);
    try {
        size_t count = 0;
        *this->db << "select count(*) from objects" >> count;
        return count;

    } catch(...) {
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

    } catch (const sqlite::sqlite_exception& e) {
        return error(io::format_str("exception: {} SQL: {}", e.what(), e.get_sql()));
    } catch(const std::exception& e) {
        return error(e.what());
    }
}


void
lurch::instance::database::restore_objects_r(std::shared_ptr<owner> object, size_t& total_restored) {

    io::info("retrieving children of: " + object->id);
    result<array_of_children> children = query_object_children(object->id);
    if(!children) {
        io::info("no children.");
        return;
    }

    io::info(std::string("number of children: ") + std::to_string(children.value().size()) );
    for(const auto &[guid, alias, type, index] : children.value()) {
        std::shared_ptr<lurch::object> child_ptr = inst->tree.create_object(
            index,
            guid,
            std::weak_ptr(object)
        );

        io::info(io::format_str("restoring object {} :: {}", guid, alias));
        ++total_restored;

        if(const auto owner_ptr = std::dynamic_pointer_cast<owner>(object->children.emplace_back(std::move(child_ptr)))) {
            io::info("owner. Restoring children.");
            restore_objects_r(owner_ptr,total_restored);
        } else {
            io::info("leaf.");
        }
    }
}


lurch::result<bool>
lurch::instance::database::restore_objects() {

    result<std::string> root_guid = query_root_guid();
    size_t total_restored_objects = 0;

    if(!root_guid) {
        root_guid = result<std::string>(object::generate_id());
        auto res = store_object(
            root_guid.value(),
            std::nullopt,
            "root",                             //alias
            object_type::ROOT,
            object_index::GENERIC_ROOT          //can be changed
        );

        if(!res) {
            return error(res.error());          //propagate error, will throw after.
        }

    } else {
        ++total_restored_objects;
        io::success("restored root object: " + root_guid.value());
    }

    std::shared_ptr<object> root_ptr = inst->tree.create_object(
        object_index::GENERIC_ROOT,
        root_guid.value(),
        std::nullopt
    );

    inst->tree.root = std::move(root_ptr);
    inst->tree.increment_object_count();

    restore_objects_r(std::dynamic_pointer_cast<owner>(inst->tree.root), total_restored_objects);

    io::success("finished restoration.");
    io::success("objects restored: " + std::to_string(total_restored_objects));

    return result<bool>(true);
}


