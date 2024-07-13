//
// Created by diago on 2024-06-05.
//

#ifndef MACRO_HPP
#define MACRO_HPP
#include <expected>
#include <variant>
#include <string>

#define LURCH_MAX_TOKEN_LENGTH 37
#define TEXT_WIDTH             70
#define PERIOD_FILLER_WIDTH    20
#define LURCH_RSA_KEYSIZE      2048
#define LURCH_CONFIG_PATH      "config.json"
#define LURCH_DEFAULT_LOGFILE  "logfile.txt"

namespace lurch {
    template<typename T>
    using result = std::expected<T, std::string>;
    using error = std::unexpected<std::string>;
    using argument_parameter = std::variant<int64_t, bool, std::string, std::monostate>;
    using empty = std::monostate;
}

#endif //MACRO_HPP
