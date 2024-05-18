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

            inst->post_message_interaction(
                user_alias,
                GUID,
                msg_result.value().empty() ? std::nullopt : std::optional(msg_result.value()),
                req.body
            );

            return true;
        }

        send_ws_notification(
            io::format_str("user {}: \nbad object access. \nerror: {}", user_alias, msg_result.error_or("?")),
            ws_notification_intent::BAD
        );
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
        const std::string& user_alias,
        const std::string &file_type,
        const crow::request &req,
        crow::response &res,
        const access_level access
    ) {

    if(req.body.empty()) {
        res.code = 400;
        return false;
    }

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    const auto upload_result = inst->tree.upload_file(GUID, req.body, file_type, access);
    if(upload_result) {
        inst->post_message_interaction(
            user_alias,
            GUID,
            std::nullopt,
            file_template(
                upload_result.value().string(),
                upload_result.value().filename().string(),
                upload_result.value().extension().string()
            )
        );

        return true;
    }

    send_ws_notification(
        io::format_str("user {}:\nbad object upload.\nerror:{}", user_alias, upload_result.error_or("?")),
        ws_notification_intent::BAD
    );

    res.code = 403;
    return false;
}