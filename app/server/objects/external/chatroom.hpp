//
// Created by diago on 2024-05-18.
//

#ifndef CHATROOM_HPP
#define CHATROOM_HPP
#include "../base/base.hpp"

namespace lurch {
class chatroom final : public leaf {
public:

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> recieve(const command &cmd) override;

    explicit chatroom(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : leaf(parent, root) {
    }

    ~chatroom() override = default;
};

} // lurch

#endif //CHATROOM_HPP
