//
// Created by diago on 4/18/2024.
//
// This header contains structs, type aliases,
// and generally just things that are not strictly functions or methods,
// but are used frequently.
//

#ifndef COMMON_HPP
#define COMMON_HPP
#include <expected>
#include <string>
#include <vector>
#include <variant>

namespace lurch {
    enum class object_type {
        GENERIC,
        CUSTOM,
        AGENT,
        EXTERNAL,
        GROUP
    };

    template<typename T>
    using result = std::expected<T, std::string>;
    using error = std::unexpected<std::string>;

    using argument_parameter = std::variant<int64_t, double, bool, std::string, std::monostate>;

    struct argument {
        std::string flag_name;
        argument_parameter parameter;
    };

    struct command {
        std::string name;
        std::vector<argument> arguments;
    };

}

#endif //COMMON_HPP
