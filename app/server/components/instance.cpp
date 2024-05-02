//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

#define LURCH_DEFAULT_BIND
#define LURCH_DEFAULT_ADDRESS "127.0.0.1"
#define LURCH_DEFAULT_PORT 8081

//
//  lurch::instance::begin() is where it all begins, literally.
//  this function is responsible for:
//  - setting up routing, database, object tree.
//  - calling database::restore_objects to set up objects that were used last time.
//
// IMPORTANT:
//  if this function fails in any way, it will throw a fatal exception.
//  if we cannot set up the server instance, we cannot continue further.

void
lurch::instance::begin() {

    std::optional<std::string> initial_user = std::nullopt;
    std::optional<std::string> initial_password = std::nullopt;
    std::string server_addr;
    uint16_t server_port = 0;

    io::print_banner();
    io::info("Initializing Lurch server instance.");


    if(!std::filesystem::exists("db/lurch_database.db")) {
        io::info("Existing database not found, one will be created.");

        initial_user = io::prompt_for("Please specify an initial username:");
        initial_password = io::prompt_for("Please specify an initial password:");
        std::cout << std::endl;

        if(initial_password.value().empty() || initial_user.value().empty()) {
            throw std::runtime_error("No default username or password provided.");
        }
    }


    this->tree.inst = this;
    this->routing.inst = this;

    result<bool> db_init = this->db.initialize(this, initial_user, initial_password);
    if(!db_init) {
        throw std::runtime_error(db_init.error());
    }

    result<bool> db_restore = this->db.restore_objects();
    if(!db_restore) {
        throw std::runtime_error(db_init.error());
    }


#if defined(LURCH_DEFAULT_BIND)
    server_addr = LURCH_DEFAULT_ADDRESS;
    server_port = LURCH_DEFAULT_PORT;
#else
    try {
        server_addr = io::prompt_for("Specify server address:");
        server_port = static_cast<uint16_t>(std::stoul(io::prompt_for("Specify port:")));
        if(server_addr.empty()) {
            throw std::exception();
        }

    } catch(...) {
        io::failure("Invalid address or port provided. Localhost:8081 will be used.");
        server_addr = "127.0.0.1";
        server_port = 8081;
    }
#endif


    std::cout << io::format_str("Attempting bind to: {}:{}", server_addr, std::to_string(server_port)) << std::endl;
    this->routing.run(server_addr, server_port); //run the server
}