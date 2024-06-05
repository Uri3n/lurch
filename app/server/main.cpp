#include <iostream>
#include <csignal>
#include <cstdlib>
#include "components/instance.hpp"
#include "util/io.hpp"

void handle_kb_interrupt(int signal) {
    std::exit(signal);
}

int main() {

    std::set_terminate(lurch::instance::handle_uncaught_exception);
    std::signal(SIGINT, handle_kb_interrupt);

    auto* inst = new lurch::instance;
    inst->begin();
    delete inst;


    return EXIT_SUCCESS;
}
