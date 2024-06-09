//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"

void
lurch::instance::router::run(
        const std::string addr,
        const uint16_t port,
        const std::optional<std::string>& ssl_cert,
        const std::optional<std::string>& ssl_key
    ) {


    CROW_ROUTE(this->app, "/")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {

        res.set_static_file_info("static/templates/login.html");
        res.code = 200;

        inst->log.write(
            "serving GET at endpoint: \"/\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });

    CROW_ROUTE(this->app, "/scripts/<string>/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string folder, std::string script_name) {

        res.code = 404;
        script_name = io::format_str("javascript/{}/{}", folder, script_name);

        if(std::filesystem::exists(script_name)) {
            const auto path = std::filesystem::path(script_name);
            if(path.has_extension() && path.extension() == ".js") {
                res.set_static_file_info(path.string());
                res.code = 200;
            }
        }

        /*                  causes output issues
        inst->log.write(
            "serving GET at endpoint: \"/scripts\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        */
        res.end();
    });

    CROW_ROUTE(this->app, "/isrunning")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {

        res.code = 200;
        res.body = "LURCH_SERVER_OK";
        res.end();
    });

    CROW_ROUTE(this->app, "/verify")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res) {

        res.code = 403;
        if(handler_verify(req, res)) {
            res.code = 200;
        }

        inst->log.write(
            "serving POST at endpoint: \"/verify\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(this->app, "/main")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM)) {
            res.set_static_file_info("static/templates/index.html");
            res.code = 200;
        }

        inst->log.write(
            "serving GET at endpoint: \"/main\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/send/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 400;
        std::string tok;

        const auto success = hdr_extract_token(req)
            .and_then([&](std::string token) {
                tok = token;
                return inst->db.query_token_context(token);
            })
            .and_then([&](std::pair<std::string, access_level> pair) {
                return result<bool>(handler_objects_send(GUID, req, res, {tok, pair.first, pair.second}));
            });

        if(success && *success == true) {
            res.code = 200;
        }

        inst->log.write(
            "serving POST at endpoint: \"/objects/send\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/upload/<string>/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID, std::string file_type){

        res.code = 403;
        const auto success = hdr_extract_token(req)
            .and_then([&](std::string token) {
                return inst->db.query_token_context(token);
            })
            .and_then([&](std::pair<std::string, access_level> ctx) {
                return result<bool>(handler_objects_upload(
                    GUID,
                    ctx.first,
                    file_type,
                    req,
                    res,
                    ctx.second
                ));
            });

        if(success && *success == true) {
            res.code = 200;
        }

        inst->log.write(
            "serving POST at endpoint: \"/objects/upload\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getdata/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM) && handler_objects_getdata(GUID, res)) {
            res.code = 200;
        }

        inst->log.write(
            "serving GET at endpoint: \"/objects/getdata\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getchildren/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM) && handler_objects_getchildren(GUID, res)) {
            res.code = 200;
        }

        inst->log.write(
            "serving GET at endpoint: \"/objects/getchildren\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getmessages/<string>/<int>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID, int index){

        auto minimum_access = access_level::HIGH;
        res.code = 403;

        const auto success = inst->tree.lookup_access_level(GUID)
            .and_then([&](access_level object_access) {
                minimum_access = object_access;
                return hdr_extract_token(req);
            })
            .and_then([&](std::string token) {
                return inst->db.query_token_context(token);
            })
            .and_then([&](std::pair<std::string, access_level> ctx) {
                if(ctx.second >= minimum_access) {
                    res.code = 200;
                    return result<bool>(handler_objects_getmessages(GUID, index, res));
                }

                return result<bool>(error("bad access level"));
            });

        if(success && *success == true) {
            res.code = 200;
        }


        inst->log.write(
            "serving GET at endpoint: \"/objects/getmessages\" :: " + std::to_string(res.code),
            log_type::INFO,
            log_noise::REGULAR
        );
        res.end();
    });


    CROW_ROUTE(app, "/ws")
    .websocket()
    .onopen([&](crow::websocket::connection& conn) {
        add_ws_connection(&conn);
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        remove_ws_connection(&conn);
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        if(verify_ws_user(&conn, data)) {
            inst->log.write("authenticated websocket connection via token.", log_type::SUCCESS, log_noise::REGULAR);
        }
        else {
            inst->log.write("websocket verification failed: " + conn.get_remote_ip(), log_type::ERROR_MINOR, log_noise::REGULAR);
        }
    });


    app.loglevel(crow::LogLevel::Critical);

    try {
        if(ssl_cert.has_value() && ssl_key.has_value()) {
            this->app
            .ssl_file(*ssl_cert, *ssl_key)
            .bindaddr(addr)
            .port(port)
            .multithreaded()
            .run();
        }

        else {
            this->app
            .bindaddr(addr)
            .port(port)
            .multithreaded()
            .run();
        }
    }
    catch(const std::exception& e) {

        std::cout << termcolor::red << "\n\n";
        std::cout << io::format_str("[!] encountered exception binding to {}:{}!", addr, port) << std::endl;
        std::cout << "[!] description: " << e.what() << std::endl;

        std::cout << "\n  this most likely occurred because an invalid address or port was provided,"
                     "\n  or because the certificate or key you provided is incorrect, or otherwise corrupted."
                     "\n  please check the validity of these items, and create a new config.json file by deleting the old one." << std::endl;

        std::cout << termcolor::reset << "\nexiting..." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}