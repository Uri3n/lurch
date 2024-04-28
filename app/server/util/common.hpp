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
