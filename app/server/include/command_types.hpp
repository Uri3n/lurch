//
// Created by diago on 2024-07-12.
//

#ifndef COMMAND_TYPES_HPP
#define COMMAND_TYPES_HPP
#include <string>
#include <vector>
#include <optional>
#include <macro.hpp>
#include <typeindex>


namespace lurch {

    template<typename T>
    concept valid_argument =
        std::is_same_v<T, std::string>  ||
        std::is_same_v<T, int64_t>      ||
        std::is_same_v<T, bool>         ||
        std::is_same_v<T, empty>;

    struct argument {
        std::string         flag_name;
        argument_parameter  parameter;
    };

    struct command {
        std::string name;
        std::vector<argument> arguments;

        template<typename ...T>
        struct param_getter {

            std::tuple<std::optional<T>...> tup;
            const command& cmd;

            template<typename U>
            std::optional<U> find_arg(const std::string& long_form, const std::string& short_form) const  {
                for(const auto &[flag_name, param] : cmd.arguments) {
                    if(long_form == flag_name || short_form == flag_name) {
                        return { std::get<U>(param) };
                    }
                }

                return std::nullopt;
            }

            decltype(tup) done() {
                return tup;
            }

            template<typename U>
            param_getter<T..., U> with(const std::string long_form, const std::string short_form) const {
                auto new_tuple = std::tuple_cat(tup, std::make_tuple(find_arg<U>(long_form, short_form)));
                return param_getter<T..., U>{new_tuple, cmd};
            }

            explicit param_getter(std::tuple<std::optional<T>...> tup, const command &cmd)
                : tup(tup), cmd(cmd) {}
        };


        template<typename T>
        param_getter<T> get(const std::string long_form, const std::string short_form) const {
            for(const auto &[flag_name, parameter] : arguments) {
                if(long_form == flag_name || short_form == flag_name) {
                    return param_getter<T>(std::make_tuple(std::get<T>(parameter)), *this);
                }
            }

            return param_getter<T>(std::make_tuple(std::nullopt), *this);
        }

        bool operator==(const command& other) const {
            return other.name == this->name;
        }

        bool operator==(const std::string& name) const {
            return name == this->name;
        }
    };

    struct formatted_argument {
        std::string long_form;
        std::string short_form;
        std::type_index type_name;
        bool required;
        std::optional<std::string> description = std::nullopt;

        formatted_argument(const std::string &long_form, const std::string &short_form, const std::type_index &type_name,
            const bool required)
            : long_form(long_form),
              short_form(short_form),
              type_name(type_name),
              required(required) {
        }
    };

    struct formatted_command {
        const std::string name;
        const std::string description;
        std::vector<formatted_argument> args;
        bool is_done = false;

        template<valid_argument T>
        formatted_command& arg(const std::string long_form, const std::string short_form, const bool required) {
            if(!is_done && long_form.size() && short_form.size()) {
                args.emplace_back(formatted_argument(
                    long_form,
                    short_form,
                    std::type_index(typeid(T)),
                    required
                ));
            }

            return *this;
        }

        formatted_command& desc(const std::string& description) {
            if(!is_done && !args.empty()) {
                args[args.size() - 1].description = description;
            }

            return *this;
        }

        formatted_command(const std::string& name, const std::string& description)
            : name(name), description(description) {}
    };


    class accepted_commands {
    private:
        std::vector<formatted_command> commands;
        bool is_done = false;
        bool match_flags(const command& passed, const formatted_command& to_compare);

    public:
        bool matches(const command& passed);
        formatted_command& add_command(const std::string& name, const std::string& description);

        void done();
        std::string help() const;
        std::string command_help(const std::string& name) const;
        bool ready() const;

        accepted_commands() = default;
    };

}


#endif //COMMAND_TYPES_HPP
