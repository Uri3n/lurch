//
// Created by diago on 4/22/2024.
//

#ifndef IO_HPP
#define IO_HPP
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <iomanip>
#include "common.hpp"
#include "../vendor/termcolor.hpp"

#define TEXT_WIDTH 70
#define PERIOD_FILLER_WIDTH 20

#define BLUE_TEXT(str) termcolor::bright_cyan << str << termcolor::reset
#define RED_TEXT(str) termcolor::red << str << termcolor::reset
#define GREEN_TEXT(str) termcolor::green << str << termcolor::reset

namespace lurch {
class io {
private:
    static std::vector<std::string> into_chunks(const std::string& str);
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

    static std::string type_to_str(const object_type type);
    static void success(const std::string& str);
    static void failure(const std::string& str);
    static void info(const std::string& str);

    static void big_success(const std::string& str);
    static void big_failure(const std::string& str);
    static void big_info(const std::string& str);

    static void print_banner();
    static std::string prompt_for(const std::string prompt);
};

} // lurch

#endif //IO_HPP
