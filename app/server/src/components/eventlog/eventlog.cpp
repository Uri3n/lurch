//
// Created by diago on 2024-06-05.
//

#include <components.hpp>


lurch::result<bool>
lurch::instance::event_log::init(const std::string& file_name) {

    log_file = std::ofstream(file_name, std::ios::app);

    if(!log_file.is_open()) {
        return error("failed to open specified log file.");
    }

    return  { true };
}

void
lurch::instance::event_log::write(const std::string &message, const log_type type, const log_noise noise) {

    ws_notification_intent notif_intent;

    switch(type) {
        case log_type::INFO:
            notif_intent = ws_notification_intent::NEUTRAL;
            io::info(message);
            break;
        case log_type::SUCCESS:
            notif_intent = ws_notification_intent::GOOD;
            io::success(message);
            break;
        default:
            notif_intent = ws_notification_intent::BAD;
            io::failure(message);
            break;
    }

    if(noise == log_noise::NOISY || noise == log_noise::REGULAR) {
        const std::string formatted_msg = format_log_message(type, message);

        {
            std::lock_guard<std::mutex> lock(log_mtx);  //it makes me cry that we have to do this
            log_file << formatted_msg;
        }

        if(noise == log_noise::NOISY) {
            inst->routing.send_ws_notification(message, notif_intent);
        }
    }

    if(type == log_type::ERROR_CRITICAL) {
        inst->set_shutdown_condition();
    }
}


std::string
lurch::instance::event_log::format_log_message(const log_type type, const std::string& message) {

    std::string log_type_str;

    switch(type) {
        case log_type::INFO:
            log_type_str = "[INFO]";
            break;
        case log_type::SUCCESS:
            log_type_str = "[SUCCESS]";
            break;
        case log_type::ERROR_CRITICAL:
            log_type_str = "[CRITICAL]";
            break;
        default:
            log_type_str = "[ERROR]";
            break;
    }

    return io::format_str("{:<11} {:<13} {}\n", log_type_str, "UTC " + curr_time(), message);
}


lurch::instance::event_log::~event_log() {
    if(log_file.is_open()) {
        log_file.close();
    }
}

