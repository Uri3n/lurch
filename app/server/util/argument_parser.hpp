//
// Created by diago on 4/18/2024.
//

#ifndef ARGUMENT_PARSER_HPP
#define ARGUMENT_PARSER_HPP
#include <expected>
#include <variant>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <map>
#include <typeindex>
#include <optional>
#include "common.hpp"


using empty = std::monostate;

namespace lurch {


    class argument_parser {
    private:
    public:

        static void                                             strip_whitespace(std::string& str);
        static std::pair<std::string, size_t>                   get_quoted_string(const std::vector<std::string>& tokens, size_t index);

        static argument_parameter                               infer_parameter_type(std::string token);

        static result<double>                                   safe_to_double(std::string token);
        static result<int64_t>                                  safe_to_signed_integer(std::string token);
        static result<bool>                                     safe_to_boolean(std::string token);

        static std::pair<std::string, std::vector<std::string>> tokenize_input(std::string& str);
        static result<command> parse(std::string raw);
    };


    template<typename T>
    concept valid_argument =
        std::is_same_v<T, std::string>  ||
        std::is_same_v<T, int64_t>      ||
        std::is_same_v<T, double>       ||
        std::is_same_v<T, bool>         ||
        std::is_same_v<T, empty>;

    struct formatted_argument {
        std::string long_form;
        std::string short_form;
        std::type_index type_name;
        bool required;

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
        std::vector<formatted_argument> args;
        bool is_done = false;

        template<valid_argument T>
        formatted_command& arg(std::string long_form, std::string short_form, bool required) {
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

        formatted_command(const std::string& name) : name(name) {}
    };


    class accepted_commands {
    private:
        std::vector<formatted_command> commands;
        bool is_done = false;
        bool match_flags(const lurch::command& passed, const formatted_command& to_compare);
    public:

        // I can't define a friend function in a .cpp file. Lol.
        friend std::ostream& operator<<(std::ostream &os, const accepted_commands &obj) {
            os << std::boolalpha << "commands: \n";
            for(const auto& command : obj.commands) {
                os << command.name + '\n';
                for(const auto& argument : command.args) {
                    os << "  " << std::setw(15) << std::left << argument.long_form;
                    os << "  " << std::setw(5) << std::left << argument.short_form;
                    os << "  " << "required: " << std::setw(5) << argument.required;
                    os << std::endl;
                }
            }

            os << std::noboolalpha;
            return os;
        }

        bool matches(const lurch::command& passed);
        formatted_command& add_command(const std::string& name);

        void done();
        bool ready() const;

        accepted_commands() = default;
    };

} //lurch

#endif //ARGUMENT_PARSER_HPP
