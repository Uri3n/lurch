//
// Created by diago on 2024-04-25.
//

#ifndef ROOT_HPP
#define ROOT_HPP
#include "../base/base.hpp"

namespace lurch {
class root final : public owner {
public:

    void shutdown(bool wipe_files) const;
    result<std::string> add_user(const std::string& user, const std::string& pass, bool admin) const;
    result<std::string> remove_user(const std::string& user) const;

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> recieve(const command &cmd) override;

    explicit root(instance* inst) : owner(std::nullopt, inst) {}
    ~root() override = default;
};

} // lurch

#endif //ROOT_HPP
