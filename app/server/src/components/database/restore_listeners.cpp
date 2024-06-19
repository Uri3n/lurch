//
// Created by diago on 2024-06-18.
//

#include <components.hpp>


void
lurch::instance::database::restore_listeners() {

    const auto listeners = query_all_listeners();
    if(!listeners) {
        io::info("No listeners to restore.");
        return;
    }


    size_t restore_count = 0;
    for(const auto &[address, guid, port, type, cert, key] : *listeners) {

        if(type != listener_type::HTTP && type != listener_type::HTTPS) {
            io::failure("Unknown listener type: " + std::to_string(static_cast<int64_t>(type)));
            continue;
        }

        if(const auto res = inst->routing.start_listener_http(address, port, guid, cert, key)) {
            io::info(io::format_str("Restored listener :: {}:{} proto: {}", address, port, io::listener_type_to_str(type)));
            ++restore_count;
        }

        else {
            io::failure(io::format_str("Failed to restore listener :: {}:{} proto: {}", address, port, io::listener_type_to_str(type)));
            io::failure("Error: " + res.error());
        }
    }

    io::info("Total restored listeners: " + std::to_string(restore_count));
}