//
// Created by diago on 4/22/2024.
//

#ifndef IO_HPP
#define IO_HPP
#include <iostream>
#include <format>
#include <string>
#include <algorithm>
#include <iomanip>
#include <common.hpp>


namespace lurch::io {

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

    bool yesno(const std::string& question);
    void success(const std::string& str);
    void failure(const std::string& str);
    void info(const std::string& str);

    void print_banner();
    std::string prompt_for(std::string prompt);

} // lurch

#endif //IO_HPP
