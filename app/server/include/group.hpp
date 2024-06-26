//
// Created by diago on 2024-05-01.
//

#ifndef GROUP_HPP
#define GROUP_HPP
#include <base.hpp>
#include <argument_parser.hpp>

namespace lurch {
class group final : public owner {
private:
    static accepted_commands commands;
    static std::unordered_map<std::string, std::function<result<std::string>(group*, reciever_context&)>> callables;

public:

    result<std::string> members(reciever_context& ctx) const;
    result<std::string> remove_member(reciever_context& ctx);
    result<std::string> add_member(reciever_context& ctx);
    result<std::string> disband(reciever_context& ctx);
    result<std::string> issue(reciever_context& ctx);
    result<std::string> groupfiles(reciever_context& ctx) const;

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string>           receive(reciever_context& ctx) override;

    static void                   init_commands();


    explicit group(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : owner(parent, root) {
    }

    ~group() override = default;
};

} // lurch

#endif //GROUP_HPP
