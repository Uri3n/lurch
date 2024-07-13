//
// Created by diago on 2024-06-18.
//

#include <baphomet.hpp>
#include <components.hpp>

lurch::baphomet::~baphomet() {
    if(delete_from_database) {
        inst->db.delete_listeners(id);
        std::thread t(std::bind(&instance::router::free_listeners, &inst->routing, this->id)); //prevent deadlock.
        t.detach();
    }
}


bool
lurch::baphomet::file_is_staged(const std::string &file_name) const {
    if(const auto file_list = inst->db.fileman_get_file_list(id)) {
        for(const auto& file : *file_list) {
            if(file_name == file) {
                return true;
            }
        }
    }

    return false;
}


lurch::result<std::string>
lurch::baphomet::delimit_command(const std::vector<std::string> &strings) {

    std::string buff;
    for(const auto& str : strings) {
        if(str.find_first_of(AGENT_DELIMITING_CHAR) != std::string::npos) {
            return error("couldn't format command, invalid character used.");
        }

        buff += str + AGENT_DELIMITING_CHAR;
    }

    return { buff };
}