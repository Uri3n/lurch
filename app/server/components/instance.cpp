//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

//
//  lurch::instance::begin() is where it all begins, literally.
//  this function is responsible for setting up the database, routing,
//  object tree, etc.
//
// IMPORTANT:
//  if this function fails in any way, it will throw a fatal exception.
//  if we cannot set up the server instance, we cannot continue further.

void lurch::instance::begin() {

    std::optional<std::string> initial_user = std::nullopt;
    std::optional<std::string> initial_password = std::nullopt;

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

    lurch::result<bool> db_init = this->db.initialize(this, initial_user, initial_password);
    if(!db_init) {
        throw std::runtime_error(db_init.error());
    }

    io::info("running server...");
    this->routing.run(); //run the server
}