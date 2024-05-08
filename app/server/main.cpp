#include <iostream>
#include <exception>
#include <csignal>
#include "components/instance.hpp"
#include "util/argument_parser.hpp"
#include "util/io.hpp"

#if defined(_MSVC_STL_VERSION) && defined(_MSVC_LANG)
    #if _MSVC_LANG > 202002L
        #define LURCH_USE_STACKTRACE
        #include <stacktrace>
    #endif
#endif

void handle_uncaught_exception() {

    std::exception_ptr exception = std::current_exception();
    std::cout << termcolor::red;

    try {
        std::rethrow_exception(exception);
    } catch(const std::exception& e) {
        std::cout << "[!] uncaught exception! what: " << e.what() << std::endl;
    } catch(...) {
        std::cout << "[!] unknown fatal exception!" << std::endl;
    }

    std::cout << termcolor::reset;

#ifdef LURCH_USE_STACKTRACE
    const std::stacktrace st = std::stacktrace::current();
    std::cout << "BEGIN STACKTRACE:" << std::endl;
    std::cout << st << std::endl;
#endif

    std::cout << "[!] exception routine finished. terminating..." << std::endl;
    std::exit(1);
}

void handle_kb_interrupt(int signal) {
    std::exit(signal);
}


int main() {

    std::set_terminate(handle_uncaught_exception);
    std::signal(SIGINT, handle_kb_interrupt);

    lurch::instance inst;
    inst.begin();

    return EXIT_SUCCESS;
}
