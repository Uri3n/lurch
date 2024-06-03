//
// Created by diago on 2024-04-25.
//

#ifndef ROOT_HPP
#define ROOT_HPP
#include "../../util/argument_parser.hpp"
#include "../base/base.hpp"

namespace lurch {
class root final : public owner {
private:
    static accepted_commands commands;
public:

    void shutdown(bool wipe_files) const;
    static void init_commands();

    result<std::string> generate_token(const command& cmd) const;
    result<std::string> create_chatroom(const command& cmd);
    result<std::string> remove_child(const command& cmd);
    result<std::string> add_user(const command& cmd) const;
    result<std::string> remove_user(const std::string& user) const;
    result<std::string> get_tokens() const;

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> recieve(const command &cmd, bool& log_if_error) override;

    explicit root(instance* inst) : owner(std::nullopt, inst) {}
    ~root() override = default;
};

} // lurch

#endif //ROOT_HPP
