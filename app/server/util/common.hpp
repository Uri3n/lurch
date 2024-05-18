//
// Created by diago on 4/18/2024.
//

#ifndef COMMON_HPP
#define COMMON_HPP
#include <expected>
#include <ostream>
#include <string>
#include <iomanip>
#include <utility>
#include <vector>
#include <utility>
#include <variant>
#include <typeindex>

namespace lurch {

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
        ROOT
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

    struct config_data {
        std::string bindaddr;
        uint16_t port;
        bool use_https;
        std::string cert_path;
        std::string key_path;
    };

    template<typename T>
    using result = std::expected<T, std::string>;
    using error = std::unexpected<std::string>;

    using argument_parameter = std::variant<int64_t, double, bool, std::string, std::monostate>;

    struct argument {
        std::string flag_name;
        argument_parameter parameter;
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

            template<typename T>
            T get(const std::string& long_form, const std::string& short_form) {
                for(const auto&[flag_name, parameter] : this->arguments) {
                    if(flag_name == long_form || flag_name == short_form) {
                        //will throw std::bad_variant_access if this doesn't exist.
                        return std::get<T>(parameter);
                    }
                }

                throw std::runtime_error("Attempted access to non-existent flag in command " + this->name);
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
    };

}

#endif //COMMON_HPP
