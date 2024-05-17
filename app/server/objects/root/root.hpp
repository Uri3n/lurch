//
// Created by diago on 2024-04-25.
//

#ifndef ROOT_HPP
#define ROOT_HPP
#include "../base/base.hpp"

namespace lurch {
class root final : public owner {
public:

    bool upload(const std::string &file, const std::string &extension) override;
    std::string recieve(const command &cmd) override;
    std::string download(const std::string &name) override;

    explicit root(instance* inst) : owner(std::nullopt, inst) {}
};

} // lurch

#endif //ROOT_HPP
