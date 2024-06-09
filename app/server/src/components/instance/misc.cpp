//
// Created by diago on 2024-06-04.
//

#include <components.hpp>

void
lurch::instance::handle_uncaught_exception() {

    const std::exception_ptr exception = std::current_exception();
    std::cout << termcolor::red;

    try {
        std::rethrow_exception(exception);
    }
    catch(const std::exception& e) {
        std::cout << "[!] uncaught exception! what: " << e.what() << std::endl;
    }
    catch(...) {
        std::cout << "[!] unknown fatal exception!" << std::endl;
    }

    std::cout << termcolor::reset;

#ifdef LURCH_USE_STACKTRACE
    const std::stacktrace st = std::stacktrace::current();
    std::cout << "BEGIN STACKTRACE:" << std::endl;
    std::cout << st << std::endl;
#endif

    std::cout << "[!] exception routine finished. terminating..." << std::endl;
    std::exit(EXIT_FAILURE);
}


void
lurch::instance::post_message_interaction(
        const std::string& sender,
        const std::string& object,
        const std::optional<std::string>& response, //can be changed to just std::string
        const std::string& message_content,
        const access_level required_access
    ) {

    routing.send_ws_object_message_update(message_content, sender, object, required_access);
    db.store_message(object, sender, message_content);

    if(response && !response->empty()) {
        routing.send_ws_object_message_update(*response, object, object, required_access);
        db.store_message(object, object, *response);
    }
}


void
lurch::instance::await_shutdown() {

    std::unique_lock<std::mutex> lock(mtx);
    io::info(io::format_str("thread with TID {}: awaiting shutdown condition.", std::this_thread::get_id()));
    shutdown_condition.wait(lock, [this]{ return shutdown; });

    routing.app.stop(); //can do additional cleanup tasks here...
}


void
lurch::instance::set_shutdown_condition() {

    std::lock_guard<std::mutex> lock (this->mtx);
    this->shutdown = true;
    this->shutdown_condition.notify_all();
}
