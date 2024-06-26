//
// Created by diago on 4/22/2024.
//

#include <io.hpp>
#include <algorithm>

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


bool
lurch::io::yesno(const std::string &question) {

    while(true) {
        std::string input = prompt_for(question + " Y/N ");
        std::ranges::transform(input, input.begin(), [](const unsigned char c) {
            return std::tolower(c);
        });

        if(input == "y" || input == "yes") {
            return true;
        }

        if(input == "n" || input == "no") {
            return false;
        }
    }
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

        case object_type::GENERIC:
            return "generic";

        default:
            return "unknown";
    }
}

std::string
lurch::io::listener_type_to_str(const listener_type type) {

    switch(type) {
        case listener_type::HTTP:
            return "HTTP";

        case listener_type::HTTPS:
            return "HTTPS";

        default:
            return "?";
    }
}

std::string
lurch::io::access_to_str(const access_level access) {

    switch(access) {
        case access_level::LOW:
            return "LOW";

        case access_level::MEDIUM:
            return "MEDIUM";

        case access_level::HIGH:
            return "HIGH";

        default:
            return "?";
    }
}


std::string
lurch::io::curr_time() {

    const auto now      = std::chrono::system_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

    // Get the total number of seconds since epoch
    const int64_t total_seconds = duration.count();
    const int64_t seconds_today = total_seconds % 86400; // 86400 seconds in a day

    const int64_t hours = seconds_today / 3600;
    const int64_t minutes = (seconds_today % 3600) / 60;
    const int64_t seconds = seconds_today % 60;

    return std::to_string(hours) + ":" +
        std::to_string(minutes) + ":" +
        std::to_string(seconds);
}


void
lurch::io::success(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|' << \
                std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " " << \
                std::left << std::right << '[' << \
                "SUCCESS" << ']' << std::setfill(' ') << std::endl;
}

void
lurch::io::failure(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|' << \
                std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " " << \
                std::left << std::right << '[' << \
                 "ERROR" << ']' << std::setfill(' ') << std::endl;
}

void
lurch::io::info(const std::string& str) {
    std::cout << '|' << std::setw(TEXT_WIDTH) << std::left << str << '|' << \
                std::setw(PERIOD_FILLER_WIDTH) << std::setfill('.') << " " << \
                std::left << std::right << '[' << \
                 "INFO" <<  ']' << std::setfill(' ') << std::endl;
}

void
lurch::io::big_info(const std::string &str) {
    if(str.empty()){
        return;
    }

    const std::vector<std::string> chunks = into_chunks(str);
    std::cout << std::endl;
    std::cout << std::format("|{:^70}|", "INFO") << std::endl;
    std::cout << std::setw(72) << std::setfill('-') << std::left << '-' << std::setfill(' ') << std::endl;

    for(const std::string& chunk : chunks) {
        std::cout << '|' << std::setw(70) << std::left << chunk << '|' << std::endl;
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
    std::cout << std::format("|{:^70}|", "ERROR") << std::endl;
    std::cout << std::setw(72) << std::setfill('-') << std::left << '-' << std::setfill(' ') << std::endl;

    for(const std::string& chunk : chunks) {
        std::cout << '|' << std::setw(70) << std::left << chunk << '|' << std::endl;
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
    std::cout << std::format("|{:^70}|", "SUCCESS") << std::endl;
    std::cout << std::setw(72) << std::setfill('-') << std::left << '-' << std::setfill(' ') << std::endl;

    for(const std::string& chunk : chunks) {
        std::cout << '|' << std::setw(70) << std::left << chunk << '|' << std::endl;
    }
    std::cout << std::endl;
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
lurch::crow_custom_logger::log(std::string message, crow::LogLevel lvl) {
    std::cout << "GOT LOG: " << message << std::endl;
}

lurch::crow_custom_logger::crow_custom_logger(const std::string &log_file_name) {
    log_file = std::ofstream(log_file_name, std::ios::app);
    if(!log_file.is_open()) {
        throw std::runtime_error("could not open/create log file.");
    }
}

lurch::crow_custom_logger::~crow_custom_logger() {
    log_file.close();
}


void
lurch::io::print_banner() {
    std::cout <<
                 R"(___       ___  ___  ________  ________  ___  ___     )" << '\n' <<
                 R"(|\  \     |\  \|\  \|\   __  \|\   ____\|\  \|\  \)" << '\n' <<
                 R"(\ \  \    \ \  \\\  \ \  \|\  \ \  \___|\ \  \\\  \   )" << '\n' <<
                 R"( \ \  \    \ \  \\\  \ \   _  _\ \  \    \ \   __  \  )" << '\n' <<
                 R"(  \ \  \____\ \  \\\  \ \  \\  \\ \  \____\ \  \ \  \ )" << '\n' <<
                 R"(   \ \_______\ \_______\ \__\\ _\\ \_______\ \__\ \__\)" << '\n' <<
                 R"(    \|_______|\|_______|\|__|\|__|\|_______|\|__|\|__|)" << "\n\n";
}
