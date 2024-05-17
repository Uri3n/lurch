//
// Created by diago on 2024-05-01.
//

#ifndef GROUP_HPP
#define GROUP_HPP
#include "../base/base.hpp"

namespace lurch {
class group final : public owner {
public:

    bool upload(const std::string &file, const std::string &extension) override;
    std::string recieve(const command &cmd) override;
    std::string download(const std::string &name) override;

    group(const std::optional<std::weak_ptr<owner>> &parent, instance * const root)
        : owner(parent, root) {
    }
};

} // lurch

#endif //GROUP_HPP
