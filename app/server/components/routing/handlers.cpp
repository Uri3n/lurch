//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"

bool
lurch::instance::router::handler_verify(const crow::request &req, crow::response &res) const {

    std::string username;

    return hdr_extract_credentials(req)
        .and_then([&](std::pair<std::string,std::string> user_info) {
            username = user_info.first;
            return inst->db.match_user(user_info.first, user_info.second);
        })
        .and_then([&](access_level access){
            res.body = database::generate_token();
            return inst->db.store_token(res.body, access, username, 12);
        })
        .has_value();
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

        //
        // TODO: remove redundant code
        //

        inst->post_message_interaction(
            user_alias,
            GUID,
            std::nullopt,
            req.body
        );

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

    return inst->db.query_object_data(GUID)
        .or_else([&](std::string err) {
            io::failure("handler_getdata: " + err);
            res.code = 404;
            return lurch::result<object_data>(std::unexpected(err));
        })
        .and_then([&](object_data data) {
            const auto& [parent,alias,type,index] = data;
            crow::json::wvalue json;

            json["parent"] = parent;
            json["alias"] = alias;
            json["type"] = io::type_to_str(type);

            res.body = json.dump();
            return lurch::result<bool>(true);
        })
        .has_value();
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

    return inst->tree.upload_file(GUID, req.body, file_type, access)
        .or_else([&](std::string err) {
            send_ws_notification(
                io::format_str("user {}:\nbad object upload.\nerror:{}", user_alias, err),
                ws_notification_intent::BAD
            );

            res.code = 403;
            return result<std::filesystem::path>(error(err));
        })
        .and_then([&](std::filesystem::path pth) {
            inst->post_message_interaction(
                user_alias,
                GUID,
                std::nullopt,
                file_template(
                    pth.string(),
                    pth.filename().string(),
                    pth.extension().string()
                )
            );

            return result<bool>(true);
        })
        .has_value();
}