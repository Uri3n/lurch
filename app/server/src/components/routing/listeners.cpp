//
// Created by diago on 2024-06-18.
//

#include <components.hpp>


lurch::http_listener::~http_listener() {

    try {
        app.stop();
        future.get(); // This can throw.
    } catch(...) {
        inst->log.write("Exception while trying to stop listener.", log_type::ERROR_MINOR, log_noise::QUIET);
    }

    inst->log.write("Freeing listener for object " + object_guid, log_type::INFO, log_noise::QUIET);
}


lurch::result<bool>
lurch::http_listener::start(
        const std::string                  address,
        const uint16_t                     port,
        const std::optional<std::string>&  certfile,
        const std::optional<std::string>&  keyfile
    ) {


    CROW_ROUTE(this->app, "/objects/send/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID){

        res.code = 403;
        if(GUID == object_guid) {

            std::string tok;
            const auto success = inst->routing.hdr_extract_token(req)
                .and_then([&](std::string token) {
                    tok = token;
                    return inst->db.query_token_context(tok);
                })
                .and_then([&](std::pair<std::string, access_level> pair) {
                    return result<bool>(inst->routing.handler_objects_send(object_guid, req, res, {tok, pair.first, pair.second}));
                });

            if(success && *success == true) {
                res.code = 200;
            }
        }


        inst->log.write(
            io::format_str("Listener for {} serving POST at \"/objects/send\" :: {}", object_guid, std::to_string(res.code)),
            log_type::INFO,
            log_noise::REGULAR
        );

        res.end();
    });


    CROW_ROUTE(this->app, "/objects/upload/<string>/<string>")
    .methods("POST"_method)([&](const crow::request& req, crow::response& res, std::string GUID, std::string file_type){

        res.code = 403;

        if(GUID == object_guid) {
            const auto success = inst->routing.hdr_extract_token(req)
                .and_then([&](std::string token) {
                    return inst->db.query_token_context(token);
                })
                .and_then([&](std::pair<std::string, access_level> ctx) {
                    return result<bool>(inst->routing.handler_objects_upload(
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
        }


        inst->log.write(
            io::format_str("Listener for {} serving POST at \"/objects/upload\" :: {}", object_guid, std::to_string(res.code)),
            log_type::INFO,
            log_noise::REGULAR
        );

        res.end();
    });


    app.loglevel(crow::LogLevel::Critical);
    if(certfile && keyfile) {
        future = app
            .ssl_file(*certfile, *keyfile)
            .bindaddr(address)
            .port(port)
            .run_async();
    } else {
        future = app
            .bindaddr(address)
            .port(port)
            .run_async();
    }


    try {
        if(!future.valid()) {
            throw std::runtime_error("Failed to start listener.");
        }

        if(future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) { //indicates exception was thrown
            future.get();
        }
    }
    catch(const std::exception& e) {
        return error(e.what());
    }
    catch(...) {
        return error("Failed to bind to specified socket.");
    }

    return { true };
}


lurch::result<bool>
lurch::instance::router::start_listener_http(
        const std::string                &address,
        const uint16_t                   port,
        const std::string                &object_guid,
        const std::optional<std::string> certfile,
        const std::optional<std::string> keyfile
    ) {

    std::lock_guard<std::mutex> lock(listeners.lock);

    auto* listener = dynamic_cast<http_listener*>(listeners.list.emplace_back(std::make_unique<http_listener>(object_guid, inst)).get());
    if(listener == nullptr) {
        return error("Failed to create listener");
    }

    return listener->start(address, port, certfile, keyfile)
        .or_else([&](std::string err) {
            listeners.list.pop_back();      // just erase the listener if we fail to bind
            return result<bool>(error(err));
        });
}


void
lurch::instance::router::free_listeners(const std::string guid) {

    std::lock_guard<std::mutex> lock(listeners.lock);

    for(auto it = listeners.list.begin(); it != listeners.list.end(); )  {
        if((*it)->object_guid == guid) {
            it = listeners.list.erase(it);
        } else {
            ++it;
        }
    }
}

