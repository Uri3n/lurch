//
// Created by diago on 2024-04-25.
//

#ifndef ROOT_HPP
#define ROOT_HPP
#include "../base/base.hpp"

namespace lurch {
class root : public owner {
private:
public:

    std::string recieve(const command &cmd) override;
    explicit root(instance* inst) : owner(std::nullopt, inst) {}
};

} // lurch

#endif //ROOT_HPP
