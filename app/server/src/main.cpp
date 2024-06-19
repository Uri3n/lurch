#include <iostream>
#include <csignal>
#include <cstdlib>
#include <components.hpp>
#include <io.hpp>

void handle_kb_interrupt(int signal) {
    std::exit(signal);
}

int main() {

    std::set_terminate(lurch::instance::handle_uncaught_exception);
    std::signal(SIGINT, handle_kb_interrupt);

    lurch::instance inst;
    inst.begin();
    return EXIT_SUCCESS;
}
