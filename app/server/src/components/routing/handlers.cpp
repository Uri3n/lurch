//
// Created by diago on 2024-05-16.
//

#include <components.hpp>

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
    const token_context& tok
) const {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    if(req.body.empty() || req.body.size() > 15000) {                                                        //check for a valid request
        return false;
    }

    reciever_context reciever_ctx;
    reciever_ctx.tok            = tok;
    reciever_ctx.address        = req.remote_ip_address;
    reciever_ctx.cmd            = argument_parser::parse(req.body).value_or(command{.name = "-"});
    reciever_ctx.message_raw    = req.body;

    const auto [response, obj_access, keep_going, log_if_error] = inst->tree.send_message(GUID, req.body, reciever_ctx);
    if(response || log_if_error) {
        inst->post_message_interaction(
            tok.alias,
            GUID,
            response.value_or(response.error()),
            req.body,
            obj_access
        );
    }

    res.body = response.value_or(std::string());
    return response.has_value();
}


bool
lurch::instance::router::handler_objects_getdata(std::string GUID, crow::response &res) const {

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    return inst->db.query_object_data(GUID)
        .or_else([&](std::string err) {
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
        for(const auto &[guid, alias, type, index] : *children) {
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
        for(const auto &[sender, body, timestamp] : *query_result) {
            crow::json::wvalue json;
            json["sender"]      = sender;
            json["body"]        = body;
            json["timestamp"]   = timestamp;
            res.body += (json.dump() + ',');
        }

        if(res.body.back() == ',') {
            res.body.pop_back();
        }

        res.body += ']';
        inst->log.write("got messages for: " + GUID, log_type::SUCCESS, log_noise::QUIET);
        return true;
    }

    inst->log.write(std::string("message query failed: ") + query_result.error(), log_type::ERROR_MINOR, log_noise::REGULAR);
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
        const access_level user_access
    ) {

    if(req.body.empty()) {
        res.code = 400;
        return false;
    }

    if(GUID == "root") {
        GUID = inst->db.query_root_guid().value_or(GUID);
    }

    return inst->tree.upload_file(GUID, req.body, file_type, user_access)
        .or_else([&](std::string err) {
            inst->log.write(
                io::format_str("user \"{}\":\nupload error.\n{}", user_alias, err),
                log_type::ERROR_MINOR,
                log_noise::NOISY
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
                ),
                user_access
            );

            return result<bool>(true);
        })
        .has_value();
}