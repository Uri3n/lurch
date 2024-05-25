//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"


void
lurch::instance::router::add_ws_connection(crow::websocket::connection* conn) {

    std::lock_guard<std::mutex> lock(websockets.lock);
    websockets.connections.emplace_back(std::make_pair(conn, std::nullopt));
    //io::info("Opened new websocket connection from " + conn->get_remote_ip());
}


void
lurch::instance::router::remove_ws_connection(crow::websocket::connection* conn) {

    std::lock_guard<std::mutex> lock(websockets.lock);
    for(auto it = websockets.connections.begin(); it != websockets.connections.end();) {
        if(it->first == conn) {
            io::info("Closing websocket connection from " + conn->get_remote_ip());
            it = websockets.connections.erase(it);
        } else {
            ++it;
        }
    }
}


bool
lurch::instance::router::verify_ws_user(crow::websocket::connection* conn, const std::string& data) {

    if(const auto token_context = inst->db.query_token_context(data)) {
        std::lock_guard<std::mutex> lock(websockets.lock);
        for(auto& [conn_ptr, access] : websockets.connections) {
            if(conn_ptr == conn) {
                access = token_context->second;
                return true;
            }
        }
    }

    return false;
}


void
lurch::instance::router::send_ws_data(
        const std::string &data,
        const bool is_binary,
        const std::optional<access_level> required_access
    ) {

    std::lock_guard<std::mutex> lock(websockets.lock);
    for(const auto& [conn, verified] : websockets.connections) {
        if(verified) {
            try {
                if(required_access.value_or(verified.value()) <= verified.value()) {
                    if(is_binary) {
                        conn->send_binary(data);
                    }
                    else {
                        conn->send_text(data);
                    }
                }
            }
            catch(...) {
                io::failure("exception while sending websocket data!");
                return;
            }
        }
    }
}


void
lurch::instance::router::send_ws_object_message_update(
        const std::string &body,
        const std::string &sender,
        std::string recipient,
        const access_level required_access
    ) {


    const auto root_guid = inst->db.query_root_guid();
    if(root_guid.has_value() && root_guid.value() == recipient) {
        recipient = "root";
    }

    crow::json::wvalue json;
    json["update-type"] = "message";
    json["body"] = body;
    json["sender"] = sender;
    json["recipient"] = recipient;

    send_ws_data(json.dump(), false, required_access);
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

    send_ws_data(json.dump(), false, std::nullopt);
}


void
lurch::instance::router::send_ws_object_delete_update(const std::string &guid) {

    crow::json::wvalue json;
    json["update-type"] = "object-delete";
    json["guid"] = guid;

    send_ws_data(json.dump(), false, std::nullopt);
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

    send_ws_data(json.dump(),false,std::nullopt);
}

