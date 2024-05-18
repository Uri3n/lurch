//
// Created by diago on 2024-04-25.
//

#include "root.hpp"
#include "../../components/instance.hpp"

lurch::result<std::filesystem::path>
lurch::root::upload(const std::string &file, const std::string &extension) {
    return error("dfdsf");
}

lurch::result<std::string>
lurch::root::recieve(const command &cmd) {

    if(cmd.name == "shutdown") {

        {
            std::lock_guard<std::mutex> lock(inst->mtx);
            inst->shutdown = true;
        }

        inst->shutdown_condition.notify_all();
    }

    return OBJECT_EMPTY_RESPONSE;
}
