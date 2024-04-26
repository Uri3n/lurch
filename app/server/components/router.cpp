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


void lurch::instance::router::add_ws_connection(crow::websocket::connection *conn) {

    std::lock_guard<std::mutex> lock(this->websockets.lock);
    this->websockets.connections.push_back(conn);
    io::info("Opened new websocket connection from " + conn->get_remote_ip());
}

void lurch::instance::router::remove_ws_connection(crow::websocket::connection* conn) {

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

void lurch::instance::router::send_ws_data(const std::string &data, const bool is_binary) {

    std::lock_guard<std::mutex> lock(this->websockets.lock);
    for(auto& conn : this->websockets.connections) {
        if(is_binary) {
            conn->send_binary(data);
        } else {
            conn->send_text(data);
        }
    }
}

inline void lurch::instance::router::send_ws_text(const std::string &data) {
    send_ws_data(data, false);
}

void lurch::instance::router::send_ws_binary(const std::string &data) {
    send_ws_data(data, true);
}

void lurch::instance::router::run(std::string addr, uint16_t port) {

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
        const auto result = hdr_extract_credentials(req);

        if(result.has_value()) {
            auto [username, password] = result.value();
            if(inst->db.match_user(username, password)) {
                res.set_static_file_info("static/templates/index.html");
                res.code = 200;
            }
        }

        io::info("serving GET at endpoint: \"/main\" :: " + std::to_string(res.code));
        res.end();
    });

    CROW_ROUTE(this->app, "/objects/send/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 200;
        res.body = std::string("got ur shit") + req.body;
        io::info("serving POST at endpoint: \"/objects/send\" :: " + std::to_string(res.code));
        res.end();
    });

    CROW_ROUTE(this->app, "/objects/getdata/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 200;
        res.body = "hello world";
        io::info("serving GET at endpoint: \"/objects/getdata\" :: " + std::to_string(res.code));
        res.end();
    });

    CROW_ROUTE(this->app, "/objects/getchildren/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 200;
        res.body = "hello world";
        io::info("serving POST at endpoint: \"/objects/getchildren\" :: " + std::to_string(res.code));
        res.end();
    });

    CROW_ROUTE(this->app, "/objects/getmessages/<string>")
    .methods("GET"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 200;
        res.body = "hello world";
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
        io::info("recieved unexpected websocket message from " + conn.get_remote_ip());
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
