//
// Created by diago on 2024-04-25.
//

#include "baphomet.hpp"

#include "../../components/instance.hpp"

lurch::result<std::filesystem::path>
lurch::baphomet::upload(const std::string &file, const std::string &extension) {

    return inst->db.fileman_create(file, extension, id, true);
}

lurch::result<std::string>
lurch::baphomet::recieve(const command &cmd) {

    inst->routing.send_ws_notification("this is a test message.", ws_notification_intent::NEUTRAL);
    return "Okay.";
}

