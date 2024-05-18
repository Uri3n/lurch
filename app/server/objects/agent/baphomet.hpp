//
// Created by diago on 2024-04-25.
//

#ifndef AGENT_HPP
#define AGENT_HPP
#include "../base/base.hpp"
#include <queue>
#include <iostream>

namespace lurch {
class baphomet final : public leaf {
private:
    std::queue<std::string> tasks;
public:

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> recieve(const lurch::command &cmd) override;

    baphomet(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : leaf(parent, root) {
    }
};

} // lurch

#endif //AGENT_HPP
