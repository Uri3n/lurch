//
// Created by diago on 2024-04-25.
//

#include "root.hpp"
#include "../../components/instance.hpp"

std::string
lurch::root::recieve(const command &cmd) {

    if(cmd.name == "shutdown") {

        {
            std::lock_guard<std::mutex> lock(inst->mtx);
            inst->shutdown = true;
        }

        inst->shutdown_condition.notify_all();
    }

    return "";
}
