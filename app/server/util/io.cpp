//
// Created by diago on 4/22/2024.
//

#include "io.hpp"

void lurch::io::success(const std::string& str) {
    std::cout << '|' << std::setw(50) << std::left << str << '|' << \
                termcolor::bright_grey << std::setw(35) << std::setfill('.') << " " << \
                std::left << std::right << termcolor::reset << '[' << \
                termcolor::green << "SUCCESS" << termcolor::reset << ']' << std::setfill(' ') << std::endl;
}

void lurch::io::failure(const std::string& str) {
    std::cout << '|' << std::setw(50) << std::left << str << '|' << \
                termcolor::bright_grey << std::setw(35) << std::setfill('.') << " " << \
                std::left << std::right << termcolor::reset << '[' << \
                termcolor::red << "FAILURE" << termcolor::reset << ']' << std::setfill(' ') << std::endl;
}

void lurch::io::info(const std::string& str) {
    std::cout << '|' << std::setw(50) << std::left << str << '|' << \
                termcolor::bright_grey << std::setw(35) << std::setfill('.') << " " << \
                std::left << std::right << termcolor::reset << '[' << \
                termcolor::blue << "INFO" << termcolor::reset << ']' << std::setfill(' ') << std::endl;
}

std::string lurch::io::prompt_for(const std::string prompt) {
    std::string input;
    std::cout << "[!] " << prompt << termcolor::bright_cyan;
    std::getline(std::cin, input);

    std::cout << termcolor::reset;
    return input;
}

void lurch::io::print_banner() {
    std::cout << termcolor::bright_cyan <<
                 R"(___       ___  ___  ________  ________  ___  ___     )" << '\n' <<
                 R"(|\  \     |\  \|\  \|\   __  \|\   ____\|\  \|\  \)" << '\n' <<
                 R"(\ \  \    \ \  \\\  \ \  \|\  \ \  \___|\ \  \\\  \   )" << '\n' <<
                 R"( \ \  \    \ \  \\\  \ \   _  _\ \  \    \ \   __  \  )" << '\n' <<
                 R"(  \ \  \____\ \  \\\  \ \  \\  \\ \  \____\ \  \ \  \ )" << '\n' <<
                 R"(   \ \_______\ \_______\ \__\\ _\\ \_______\ \__\ \__\)" << '\n' <<
                 R"(    \|_______|\|_______|\|__|\|__|\|_______|\|__|\|__|)" <<
                 termcolor::reset << "\n\n";
}
