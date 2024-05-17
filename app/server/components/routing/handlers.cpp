//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"

bool
lurch::instance::router::handler_verify(const crow::request &req, crow::response &res) const {

    const auto result = hdr_extract_credentials(req);

    if(result.has_value()) {
        const auto [username, password] = result.value();
        if(const auto user_result = inst->db.match_user(username, password)) {

            const std::string auth_token = database::generate_token();
            const auto store_auth_token = inst->db.store_token(auth_token, user_result.value(), username);

            if(!store_auth_token) {
                io::failure("Failed to store token!");
                io::failure("error: " + store_auth_token.error_or("-"));
                return false;
            }

            res.body = auth_token;
            return true;
        }
    }

    return false;
}


bool
lurch::instance::router::handler_objects_send(
    std::string GUID,
    const crow::request& req,
    crow::response& res,
    const std::string& user_alias,
    const access_level user_access
) {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    if(!req.body.empty() && req.body.size() < 200) {                                                    //check for a valid request
        const auto msg_result = inst->tree.send_message(GUID, req.body, user_access);                   //send the message to the object
        if(msg_result.has_value()) {
            res.body = msg_result.value();

            io::success("successfully parsed command: " + req.body);
            io::success("responsible object: " + GUID);

            inst->db.store_message(GUID, req.remote_ip_address, req.body);                              //we potentially need to store two messages here: client->server and server->client
            send_ws_object_message_update(                                                              //send message update to websocket clients
                req.body,
                user_alias,
                GUID
            );

            if(!msg_result.value().empty()) {
                inst->db.store_message(GUID, GUID, msg_result.value());                                 //object may decide to not return a message, in this case don't store or send anything.
                send_ws_object_message_update(msg_result.value(), GUID, GUID);
            }

            return true;
        }
        res.code = 403;
        return false;
    }

    res.code = 400;
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

        res.body = json.dump();
        return true;

    } else {
        io::failure(io::format_str("getdata request for {} failed.", GUID));
        io::failure("error: " + data_result.error());
        res.code = 404;
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

    res.code = 404;
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
    res.code = 404;
    return false;
}


bool
lurch::instance::router::handler_objects_upload(
        std::string GUID,
        const std::string &file_type,
        const crow::request &req,
        crow::response &res,
        const access_level access
    ) const {

    if(req.body.empty()) {
        res.code = 400;
        return false;
    }

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    return inst->tree.upload_file(
        GUID,
        req.body,
        file_type,
        access
    );
}