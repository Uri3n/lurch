//
// Created by diago on 2024-04-25.
//

#include "agent.hpp"

#include "../../components/instance.hpp"

std::string lurch::baphomet::upload(const std::string &file, const std::string &extension) {
    return "blabsda";
}

std::string
lurch::baphomet::recieve(const command &cmd) {

    inst->routing.send_ws_notification("this is a test message.", ws_notification_intent::NEUTRAL);
    return "Okay.";
}
