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
#include <common.hpp>
#include <io.hpp>
#include <templating.hpp>
#include <command_types.hpp>

namespace lurch::argument_parser {
    void strip_whitespace(std::string& str);
    std::pair<std::string, size_t> get_quoted_string(const std::vector<std::string>& tokens, size_t index);
    argument_parameter infer_parameter_type(std::string token);
    result<int64_t> safe_to_signed_integer(std::string token);
    result<bool> safe_to_boolean(std::string token);
    std::pair<std::string, std::vector<std::string>> tokenize_input(std::string& str);
    result<command> parse(std::string raw);
} //lurch

#endif //ARGUMENT_PARSER_HPP
