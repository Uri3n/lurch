//
// Created by diago on 2024-07-12.
//

#include <basic.hpp>

std::string
lurch::type_to_str(const object_type type) {

    switch(type) {
        case object_type::NONE:     return "none";
        case object_type::ROOT:     return "root";
        case object_type::GROUP:    return "group";
        case object_type::AGENT:    return "agent";
        case object_type::EXTERNAL: return "external";
        case object_type::GENERIC:  return "generic";
        default:                    return "unknown";
    }
}

std::string
lurch::listener_type_to_str(const listener_type type) {

    switch(type) {
        case listener_type::HTTP:  return "HTTP";
        case listener_type::HTTPS: return "HTTPS";
        default:                   return "?";
    }
}

std::string
lurch::access_to_str(const access_level access) {

    switch(access) {
        case access_level::LOW:     return "LOW";
        case access_level::MEDIUM:  return "MEDIUM";
        case access_level::HIGH:    return "HIGH";
        default:                    return "?";
    }
}

std::string
lurch::curr_time() {

    const auto now      = std::chrono::system_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

    const int64_t total_seconds = duration.count();
    const int64_t seconds_today = total_seconds % 86400;
    const int64_t hours         = seconds_today / 3600;
    const int64_t minutes       = seconds_today % 3600 / 60;
    const int64_t seconds       = seconds_today % 60;

    return std::to_string(hours) + ":" +
        std::to_string(minutes)  + ":" +
        std::to_string(seconds);
}
