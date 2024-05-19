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

        io::info("serving GET at endpoint: \"/\" :: " + std::to_string(res.code));
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

        io::info("serving POST at \"/verify\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/main")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM)) {
            res.set_static_file_info("static/templates/index.html");
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/main\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/send/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 403;
        if(const auto result = inst->db.query_token_context(hdr_extract_token(req).value_or("-"))) {
            if(handler_objects_send(GUID, req, res, result.value().first, result.value().second)) {
                res.code = 200;
            }
        }

        io::info("serving POST at endpoint: \"/objects/send\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/upload/<string>/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID, std::string file_type){

        res.code = 403;

        const auto token_context = inst->db.query_token_context(hdr_extract_token(req).value_or("-"));
        if(token_context.has_value()) {
            if(handler_objects_upload(
                GUID,
                token_context.value().first,
                file_type,
                req,
                res,
                token_context.value().second
            )) {
                res.code = 200;
            }
        }

        io::info("serving POST at endpoint: \"/objects/send\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getdata/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM) && handler_objects_getdata(GUID, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/objects/getdata\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getchildren/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM) && handler_objects_getchildren(GUID, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/objects/getchildren\" :: " + std::to_string(res.code));
        res.end();
    });


    CROW_ROUTE(this->app, "/objects/getmessages/<string>/<int>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID, int index){

        res.code = 403;
        if(verify_token(req, access_level::MEDIUM) && handler_objects_getmessages(GUID, index, res)) {
            res.code = 200;
        }

        io::info("serving GET at endpoint: \"/objects/getmessages\" :: " + std::to_string(res.code));
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
        if(!is_binary && verify_ws_user(&conn, data)) {
            io::success("authenticated websocket connection via token.");
        }
        else {
            io::failure("websocket verification failed: " + conn.get_remote_ip());
        }
    });


    app.loglevel(crow::LogLevel::Critical);

    try {
        if(ssl_cert.has_value() && ssl_key.has_value()) {
            this->app
            .ssl_file(ssl_cert.value(), ssl_key.value())
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
        std::exit(1);
    }
}