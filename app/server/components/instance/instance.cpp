//
// Created by diago on 2024-04-24.
//

#include "../instance.hpp"

void
lurch::instance::begin() {

    std::optional<std::string> initial_user = std::nullopt;
    std::optional<std::string> initial_password = std::nullopt;

    io::print_banner();
    io::info("Initializing Lurch server instance.");


    //
    // check for existing database
    //

    if(!std::filesystem::exists("db/lurch_database.db")) {
        io::info("Existing database not found, one will be created.");

        if(!std::filesystem::exists("db/") && !std::filesystem::create_directory("db/")) {
            throw std::runtime_error("failed to create database at directory \"/db\"");
        }

        initial_user        = io::prompt_for("Please specify an initial username:");
        initial_password    = io::prompt_for("Please specify an initial password:");
        std::cout << std::endl;

        if(initial_password->empty() || initial_user->empty()) {
            throw std::runtime_error("No default username or password provided.");
        }
    }


    //
    // initialize server config
    //

    const auto config = init_config_data();
    if(!config) {
        throw std::runtime_error(config.error_or("failed to initialize server config."));
    }


    //
    // initialize other members
    //

    tree.inst       = this;
    routing.inst    = this;
    log.inst        = this;


    if(const auto db_init = db.initialize(this, initial_user, initial_password); !db_init) {
        throw std::runtime_error(db_init.error());
    }

    if(const auto db_restore = db.restore_objects(); !db_restore) {
        throw std::runtime_error(db_restore.error());
    }

    if(const auto logger_init = log.init(LURCH_DEFAULT_LOGFILE); !logger_init) {
        throw std::runtime_error(logger_init.error());
    }

    db.delete_old_tokens();


    //
    // set up worker thread and attempt bind to specified addr/port
    //

    log.write(
        io::format_str( "\nAttempting bind to: {}:{}", config->bindaddr, std::to_string(config->port)),
        log_type::INFO,
        log_noise::REGULAR
    );

    std::thread worker([&] {

        std::set_terminate(handle_uncaught_exception);
        if(config->use_https) {
            routing.run(
                config->bindaddr,
                config->port,
                config->cert_path,
                config->key_path
            );
        }
        else {
            routing.run(
                config->bindaddr,
                config->port,
                std::nullopt,
                std::nullopt
            );
        }
    });


    //
    // await server shutdown
    //

    await_shutdown();
    worker.join();
}