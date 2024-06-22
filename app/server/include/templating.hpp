//
// Created by diago on 2024-06-21.
//

#ifndef TEMPLATING_HPP
#define TEMPLATING_HPP
#include <string>
#include <crow.h>
#include <io.hpp>

namespace lurch {
    class templates {
    public:

        static std::string terminal_media(const std::string& uri_path, const std::string& base_name, const std::string& extension);
        static std::string command_list(const std::string& header, const std::vector<std::pair<std::string, std::string>>& commands);
        static std::string flag_list(const std::string& header, const std::vector<flag_descriptor>& flag_descriptors);
    };
}

#endif //TEMPLATING_HPP
