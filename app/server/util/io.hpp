//
// Created by diago on 4/22/2024.
//

#ifndef IO_HPP
#define IO_HPP
#include <iostream>
#include <format>
#include <string>
#include <iomanip>
#include "../vendor/termcolor.hpp"

namespace lurch {
class io {
public:

    template<typename ... Args>
    static std::string format_str(const std::format_string<Args...> fmt, Args... args) {
        std::string output;

        try {
            output = std::vformat(fmt.get(), std::make_format_args(args...));
        } catch(const std::format_error& e) {
            output = std::string("FORMAT ERROR: ") + e.what();
        } catch(...) {
            output = "UNKNOWN FORMATTING ERROR";
        }

        return output;
    }

    static void success(const std::string& str);
    static void failure(const std::string& str);
    static void info(const std::string& str);

    static void print_banner();
    static std::string prompt_for(const std::string prompt);
};

} // lurch

#endif //IO_HPP
