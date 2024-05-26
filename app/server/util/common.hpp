//
// Created by diago on 4/18/2024.
//

#ifndef COMMON_HPP
#define COMMON_HPP
#include <expected>
#include <ostream>
#include <string>
#include <iomanip>
#include <optional>
#include <utility>
#include <vector>
#include <utility>
#include <variant>
#include <typeindex>

#define LURCH_MAX_TOKEN_LENGTH 37

namespace lurch {

    template<typename T>
    using result = std::expected<T, std::string>;
    using error = std::unexpected<std::string>;

    using argument_parameter = std::variant<int64_t, double, bool, std::string, std::monostate>;

    enum class access_level : int32_t {
        LOW,
        MEDIUM,
        HIGH
    };

    enum class object_type : int64_t {
        NONE,
        GROUP,
        AGENT,
        EXTERNAL,
        ROOT,
        GENERIC
    };

    enum class object_index : int64_t {
        BAPHOMET,
        GENERIC_GROUP,
        GENERIC_CHATROOM,
        GENERIC_ROOT,
    };

    enum class ws_notification_intent : uint16_t {
        NEUTRAL,
        BAD,
        GOOD
    };

    template<size_t num_extensions>
    struct filetype_pair {
        std::array<std::string_view, num_extensions> extensions;
        std::string_view html;

        consteval filetype_pair(
            decltype(extensions) ext,
            std::string_view html
            ) : extensions(ext), html(html) {}
    };

    struct token_context {
        std::string token;
        std::string alias;
        access_level access = access_level::LOW;
    };

    struct full_token_data {
        std::string token;
        std::string expiration;
        std::string alias;
        access_level access = access_level::LOW;
    };

    struct config_data {
        std::string bindaddr;
        uint16_t port;
        bool use_https;
        std::string cert_path;
        std::string key_path;
    };

    struct argument {
        std::string flag_name;
        argument_parameter parameter;
    };

    struct search_ctx {
        result<std::string> response;
        access_level obj_access = access_level::LOW;
        bool keep_going = true;
        bool log_if_error = true;
    };

    template<typename T> requires std::is_invocable_v<T>
    class defer_wrapper {
        T callable;
    public:

        auto call() -> decltype(callable()) {
            return callable();
        }

        explicit defer_wrapper(T func) : callable(func) {}
        ~defer_wrapper() { callable(); }
    };


    template<typename T>
    defer_wrapper<T> defer(T callable) {
        return defer_wrapper<T>(callable);
    }

    struct command {
        private:

            struct print_value {
                std::ostream& os;

                //exclusively used by the stream overload function, alongside std::visit.
                void operator()(int64_t value) const {          os << "[i64]: " << value; }
                void operator()(double value) const {           os << "[double]: " << value; }
                void operator()(bool value) const {             os << std::boolalpha << "[bool]: " << value << std::noboolalpha; }
                void operator()(std::string value) const {      os << "[string]: " << value; }
                void operator()(std::monostate value) const {   os << "[empty]"; }

                explicit print_value(std::ostream& os) : os(os) {}
            };

        public:
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

            friend std::ostream & operator<<(std::ostream &os, const command &obj) {
                os << obj.name << std::endl;
                for(const auto&[flag_name, parameter] : obj.arguments) {
                    os << "  " << std::setw(15) << std::left << flag_name;
                    std::visit(print_value(os), parameter);
                    os << std::endl;
                }

                return os;
            }


            bool operator==(const command& other) const {
                return other.name == this->name;
            }

            bool operator==(const std::string& name) const {
                return name == this->name;
            }
    };


}

#endif //COMMON_HPP
