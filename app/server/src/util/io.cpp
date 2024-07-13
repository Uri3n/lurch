//
// Created by diago on 4/22/2024.
//

#include <io.hpp>


bool
lurch::io::yesno(const std::string &question) {

    while(true) {
        std::string input = prompt_for(question + " Y/N ");
        std::ranges::transform(input, input.begin(), [](const unsigned char c) {
            return std::tolower(c);
        });

        if(input == "y" || input == "yes")
            return true;

        if(input == "n" || input == "no")
            return false;
    }
}

void
lurch::io::success(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|'
        << std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " "
        << std::left << std::right << '['  << "SUCCESS" << ']'
        << std::setfill(' ') << '\n';
}

void
lurch::io::failure(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|'
        << std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " "
        << std::left << std::right << '['  << "ERROR" << ']'
        << std::setfill(' ') << '\n';
}

void
lurch::io::info(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|'
        << std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.')  << " "
        << std::left << std::right << '['  << "INFO" <<  ']'
        << std::setfill(' ') << '\n';
}

std::string
lurch::io::prompt_for(const std::string prompt) {
    std::string input;
    std::cout << "[!] " << prompt;
    std::getline(std::cin, input);

    std::cout << std::flush;
    return input;
}

void
lurch::io::print_banner() {
    std::cout <<
         R"(___       ___  ___  ________  ________  ___  ___     )"  << '\n' <<
         R"(|\  \     |\  \|\  \|\   __  \|\   ____\|\  \|\  \)"     << '\n' <<
         R"(\ \  \    \ \  \\\  \ \  \|\  \ \  \___|\ \  \\\  \   )" << '\n' <<
         R"( \ \  \    \ \  \\\  \ \   _  _\ \  \    \ \   __  \  )" << '\n' <<
         R"(  \ \  \____\ \  \\\  \ \  \\  \\ \  \____\ \  \ \  \ )" << '\n' <<
         R"(   \ \_______\ \_______\ \__\\ _\\ \_______\ \__\ \__\)" << '\n' <<
         R"(    \|_______|\|_______|\|__|\|__|\|_______|\|__|\|__|)" << "\n\n";
}
