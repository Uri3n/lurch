//
// Created by diago on 4/18/2024.
//

#ifndef ARGUMENT_PARSER_HPP
#define ARGUMENT_PARSER_HPP
#include <expected>
#include <variant>
#include <vector>
#include <sstream>
#include "common.hpp"

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
} //lurch::util

#endif //ARGUMENT_PARSER_HPP
