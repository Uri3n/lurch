//
// Created by diago on 2024-04-25.
//

#ifndef AGENT_HPP
#define AGENT_HPP
#include "../base/base.hpp"
#include <queue>
#include <iostream>

namespace lurch {
class agent : public leaf {
private:
    std::queue<std::string> tasks;
public:

    std::string recieve(const lurch::command &cmd) override;
    agent(const std::optional<owner *> &parent, instance* const root)
        : leaf(parent, root) {
    }
};

} // lurch

#endif //AGENT_HPP
