//
// Created by diago on 4/22/2024.
//

#include "io.hpp"

std::vector<std::string>
lurch::io::into_chunks(const std::string& str) {
    size_t cur_chunk_begin = 0;
    std::vector<std::string> chunks;

    for(size_t i = 0; i < str.size(); i++) {
        if(i && i % TEXT_WIDTH == 0) {
            chunks.emplace_back(str.substr(cur_chunk_begin, i - cur_chunk_begin));
            cur_chunk_begin = i;
        }
    }

    if(cur_chunk_begin < str.size()) {
        chunks.emplace_back(str.substr(cur_chunk_begin));
    }

    return chunks;
}

std::string
lurch::io::type_to_str(const object_type type) {

    switch(type) {
        case object_type::NONE:
            return "none";
        case object_type::ROOT:
            return "root";
        case object_type::GROUP:
            return "group";
        case object_type::AGENT:
            return "agent";
        case object_type::EXTERNAL:
            return "external";
        default:
            return "unknown";
    }
}

void
lurch::io::success(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|' << \
                termcolor::bright_grey << std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " " << \
                std::left << std::right << termcolor::reset << '[' << \
                termcolor::green << "SUCCESS" << termcolor::reset << ']' << std::setfill(' ') << std::endl;
}

void
lurch::io::failure(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|' << \
                termcolor::bright_grey << std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " " << \
                std::left << std::right << termcolor::reset << '[' << \
                termcolor::red << "ERROR" << termcolor::reset << ']' << std::setfill(' ') << std::endl;
}

void
lurch::io::info(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|' << \
                termcolor::bright_grey << std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " " << \
                std::left << std::right << termcolor::reset << '[' << \
                termcolor::blue << "INFO" << termcolor::reset << ']' << std::setfill(' ') << std::endl;
}

void
lurch::io::big_info(const std::string &str) {
    if(str.empty()){
        return;
    }

    const std::vector<std::string> chunks = into_chunks(str);
    std::cout << std::endl;
    std::cout << BLUE_TEXT(std::format("|{:^70}|", "INFO")) << std::endl;
    std::cout << std::setw(72) << std::setfill('-') << std::left << '-' << std::setfill(' ') << std::endl;

    for(const std::string& chunk : chunks) {
        std::cout << BLUE_TEXT('|') << std::setw(70) << std::left << chunk << BLUE_TEXT('|') << std::endl;
    }
    std::cout << std::endl;
}

void
lurch::io::big_failure(const std::string &str) {
    if(str.empty()){
        return;
    }

    const std::vector<std::string> chunks = into_chunks(str);
    std::cout << std::endl;
    std::cout << RED_TEXT(std::format("|{:^70}|", "ERROR")) << std::endl;
    std::cout << std::setw(72) << std::setfill('-') << std::left << '-' << std::setfill(' ') << std::endl;

    for(const std::string& chunk : chunks) {
        std::cout << RED_TEXT('|') << std::setw(70) << std::left << chunk << RED_TEXT('|') << std::endl;
    }
    std::cout << std::endl;
}

void
lurch::io::big_success(const std::string &str) {
    if(str.empty()){
        return;
    }

    const std::vector<std::string> chunks = into_chunks(str);
    std::cout << std::endl;
    std::cout << GREEN_TEXT(std::format("|{:^70}|", "SUCCESS")) << std::endl;
    std::cout << std::setw(72) << std::setfill('-') << std::left << '-' << std::setfill(' ') << std::endl;

    for(const std::string& chunk : chunks) {
        std::cout << GREEN_TEXT('|') << std::setw(70) << std::left << chunk << GREEN_TEXT('|') << std::endl;
    }
    std::cout << std::endl;
}


std::string
lurch::io::prompt_for(const std::string prompt) {
    std::string input;
    std::cout << "[!] " << prompt << termcolor::bright_cyan;
    std::getline(std::cin, input);

    std::cout << termcolor::reset;
    return input;
}


void
lurch::io::print_banner() {
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
