//
// Created by diago on 2024-06-21.
//

#ifndef TEMPLATING_HPP
#define TEMPLATING_HPP
#include <string>
#include <crow.h>
#include <io.hpp>

namespace lurch::templates {
    std::string terminal_media(const std::string& uri_path, const std::string& base_name, const std::string& extension);
    std::string command_list(const std::string& header, const std::vector<std::pair<std::string, std::string>>& commands);
    std::string flag_list(const std::string& header, const std::vector<flag_descriptor>& flag_descriptors);
    std::string generic_header_with_content(const std::string& header, const std::string& content);
}

#endif //TEMPLATING_HPP
