//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

lurch::result<std::pair<std::string, std::string>> lurch::instance::router::hdr_extract_credentials(const crow::request &req) {

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
        return lurch::error("Invalid credentials provided.");
    }
}


void lurch::instance::router::run() {

    CROW_ROUTE(this->app, "/")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {
        res.set_static_file_info("static/templates/login.html");
        res.code = 200;
        res.end();
    });

    CROW_ROUTE(this->app, "/main")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res) {
        res.code = 403;
        const auto result = hdr_extract_credentials(req);

        if(result.has_value()) {
            auto [username, password] = result.value();
            if(inst->db.match_user(username, password)) {
                res.set_static_file_info("static/templates/index.html");
                res.code = 200;
            }
        }

        res.end();
    });

    app.loglevel(crow::LogLevel::Critical);
    this->app
    .bindaddr("127.0.0.1")
    .port(8081)
    .multithreaded()
    .run();
}