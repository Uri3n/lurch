//
// Created by diago on 2024-04-25.
//

#ifndef ROOT_HPP
#define ROOT_HPP
#include <argument_parser.hpp>
#include <base.hpp>

namespace lurch {
class root final : public owner {
private:
    static accepted_commands commands;
    static std::unordered_map<std::string, std::function<result<std::string>(root*, reciever_context&)>> callables;

public:

    static void init_commands();

    result<std::string> get_tokens() const;
    result<std::string> get_listeners() const;

    result<std::string> shutdown(reciever_context& ctx) const;
    result<std::string> remove_user(reciever_context& ctx) const;
    result<std::string> generate_token(reciever_context& ctx) const;
    result<std::string> create(reciever_context& ctx);
    result<std::string> remove_child(reciever_context& ctx);
    result<std::string> add_user(reciever_context& ctx) const;
    result<std::string> delete_token(reciever_context& ctx) const;

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> receive(reciever_context& ctx) override;

    explicit root(instance* inst) : owner(std::nullopt, inst) {}
    ~root() override = default;
};

} // lurch

#endif //ROOT_HPP
