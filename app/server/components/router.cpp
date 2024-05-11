//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

lurch::result<std::pair<std::string, std::string>>
lurch::instance::router::hdr_extract_credentials(const crow::request &req) {

    try {
        std::string encoded_creds = req.get_header_value("Authorization").substr(6);
        if(encoded_creds.empty()) {
            throw std::exception();
        }

        std::string decoded_creds  = crow::utility::base64decode(encoded_creds, encoded_creds.size());

        size_t found = decoded_creds.find(':');
        if(found == std::string::npos) {
            throw std::exception();
        }

        return std::make_pair(decoded_creds.substr(0, found), decoded_creds.substr(found+1));
    } catch(...) {
        return error("Invalid credentials provided.");
    }
}


void
lurch::instance::router::add_ws_connection(crow::websocket::connection *conn) {

    std::lock_guard<std::mutex> lock(this->websockets.lock);
    this->websockets.connections.push_back(conn);
    //io::info("Opened new websocket connection from " + conn->get_remote_ip());
}


void
lurch::instance::router::remove_ws_connection(crow::websocket::connection* conn) {

    std::lock_guard<std::mutex> lock(this->websockets.lock);
    for(auto it = websockets.connections.begin(); it != websockets.connections.end();) {
        if(*it == conn) {
            io::info("Closing websocket connection from " + conn->get_remote_ip());
            it = websockets.connections.erase(it);
        } else {
            ++it;
        }
    }
}


void
lurch::instance::router::send_ws_data(const std::string &data, const bool is_binary) {

    std::lock_guard<std::mutex> lock(this->websockets.lock);
    for(auto conn : this->websockets.connections) {
        try {
            if(is_binary) {
                conn->send_binary(data);
            } else {
                conn->send_text(data);
            }
        } catch(...) {
            io::failure("exception while sending websocket data!");
            return;
        }
    }
}


void
lurch::instance::router::send_ws_text(const std::string &data) {
    send_ws_data(data, false);
}


void
lurch::instance::router::send_ws_binary(const std::string &data) {
    send_ws_data(data, true);
}


void
lurch::instance::router::send_ws_object_message_update(const std::string &body, const std::string &sender, std::string recipient) {

    const auto root_guid = inst->db.query_root_guid();
    if(root_guid.has_value() && root_guid.value() == recipient) {
        recipient = "root";
    }

    crow::json::wvalue json;
    json["update-type"] = "message";
    json["body"] = body;
    json["sender"] = sender;
    json["recipient"] = recipient;

    send_ws_text(json.dump());
}


void
lurch::instance::router::send_ws_object_create_update(
    const std::string &guid,
    std::string parent, const
    std::string& alias,
    const object_type type
    ) {

    const auto root_guid = inst->db.query_root_guid();
    if(root_guid.has_value() && root_guid.value() == parent) {
        parent = "root";
    }

    crow::json::wvalue json;
    json["update-type"] = "object-create";
    json["guid"] = guid;
    json["parent"] = parent;
    json["alias"] = alias;
    json["type"] = io::type_to_str(type);

    send_ws_text(json.dump());
}


void
lurch::instance::router::send_ws_object_delete_update(const std::string &guid) {

    crow::json::wvalue json;
    json["update-type"] = "object-delete";
    json["guid"] = guid;

    send_ws_text(json.dump());
}


void
lurch::instance::router::send_ws_notification(const std::string &message, const ws_notification_intent intent) {

    crow::json::wvalue json;
    json["update-type"] = "notification";
    json["body"] = message;

    switch(intent) {
        case ws_notification_intent::GOOD:
            json["intent"] = "good";
            break;
        case ws_notification_intent::BAD:
            json["intent"] = "bad";
            break;
        default:
            json["intent"] = "neutral";
            break;
    }

    send_ws_text(json.dump());
}


bool
lurch::instance::router::handler_main(const crow::request &req, crow::response &res) const {

    const auto result = hdr_extract_credentials(req);
    if(result.has_value()) {
        auto [username, password] = result.value();
        if(inst->db.match_user(username, password)) {
            res.set_static_file_info("static/templates/index.html");
            return true;
        }
    }

    return false;
}


bool
lurch::instance::router::handler_objects_send(std::string GUID, const crow::request& req, crow::response& res) {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    if(!req.body.empty() && req.body.size() < 200) {                                                    //check for a valid request
        const auto msg_result = inst->tree.send_message(GUID, req.body);                                //send the message
        if(msg_result.has_value()) {
            res.body = msg_result.value();

            io::success("successfully parsed command: " + req.body);
            io::success("responsible object: " + GUID);

            inst->db.store_message(GUID, req.remote_ip_address, req.body);                              //we potentially need to store two messages here: client->server and server->client
            send_ws_object_message_update(req.body, req.remote_ip_address, GUID);                       //send websocket update for the message.

            if(!msg_result.value().empty()) {
                inst->db.store_message(GUID, GUID, msg_result.value());                                 //object may decide to not return a message, in this case don't store or send anything.
                send_ws_object_message_update(msg_result.value(), GUID, GUID);
            }

            return true;
        }
        io::failure("Invalid message sent to object.");
    }

    return false;
}


bool
lurch::instance::router::handler_objects_getdata(std::string GUID, crow::response &res) const {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    if(const auto data_result = inst->db.query_object_data(GUID)) {
        const auto& [parent,alias,type,index] = data_result.value();

        crow::json::wvalue json;
        json["parent"] = parent;
        json["alias"] = alias;
        json["type"] = io::type_to_str(type);

        res.code = 200;
        res.body = json.dump();
        return true;

    } else {
        io::failure(io::format_str("getdata request for {} failed.", GUID));
        io::failure("error: " + data_result.error());
        return false;
    }
}

bool
lurch::instance::router::handler_objects_getchildren(std::string GUID, crow::response &res) const {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    result<array_of_children> children = inst->db.query_object_children(GUID);
    if(children) {
        res.body = '[';
        for(const auto &[guid, alias, type, index] : children.value()) {
            crow::json::wvalue json;

            json["guid"] = guid;
            json["alias"] = alias;
            json["type"] = io::type_to_str(type);

            res.body += (json.dump() + ',');
        }

        if(res.body.back() == ',') {
            res.body.pop_back();
        }

        res.body += ']';
        return true;
    }

    return false;
}


bool
lurch::instance::router::handler_objects_getmessages(std::string GUID, const int message_index, crow::response &res) const {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    const auto query_result = inst->db.query_object_messages(GUID, message_index);
    if(query_result.has_value()) {

        res.body = '[';
        for(const auto &[sender, body, timestamp] : query_result.value()) {
            crow::json::wvalue json;
            json["sender"] = sender;
            json["body"] = body;
            json["timestamp"] = timestamp;
            res.body += (json.dump() + ',');
        }

        if(res.body.back() == ',') {
            res.body.pop_back();
        }

        res.body += ']';
        io::success("got messages for: " + GUID);
        return true;
    }

    io::failure("message query failed.");
    io::failure("error: " + query_result.error());
    return false;
}


void
lurch::instance::router::run(std::string addr, uint16_t port) {

    CROW_ROUTE(this->app, "/")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {
        res.set_static_file_info("static/templates/login.html");
        res.code = 200;

        io::info("serving GET at endpoint: \"/\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/isrunning")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {
        res.code = 200;
        res.body = "LURCH_SERVER_OK";
        res.end();
    });


    CROW_ROUTE(this->app, "/main")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {
        res.code = 403;
        if(handler_main(req, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/main\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/send/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID){
        res.code = 400;
        if(handler_objects_send(GUID, req, res)) {
            res.code = 200;
        }

        io::info("serving POST at endpoint: \"/objects/send\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getdata/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){
        res.code = 404;
        if(handler_objects_getdata(GUID, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/objects/getdata\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getchildren/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){
        res.code = 404;
        if(handler_objects_getchildren(GUID, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/objects/getchildren\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getmessages/<string>/<int>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID, int index){

        res.code = 404;
        if(handler_objects_getmessages(GUID, index, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/objects/getmessages\" :: " + std::to_string(res.code));
        res.end();
    });


    //
    // Primary websocket endpoint.
    // Note that we should never recieve messages on this endpoint.
    //

    CROW_ROUTE(app, "/ws")
    .websocket()
    .onopen([&](crow::websocket::connection& conn) {
        add_ws_connection(&conn);
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        remove_ws_connection(&conn);
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        io::failure("recieved unexpected websocket message from " + conn.get_remote_ip());
        if(!is_binary) {
            io::info("message: " + data);
        }
    });

    app.loglevel(crow::LogLevel::Critical);
    this->app
    .bindaddr(addr)
    .port(port)
    .multithreaded()
    .run();
}
