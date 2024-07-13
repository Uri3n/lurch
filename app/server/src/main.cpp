#include <cstdlib>
#include <components.hpp>

int main() {
    std::set_terminate(lurch::instance::handle_uncaught_exception);
    lurch::instance inst;
    inst.begin();

    return EXIT_SUCCESS;
}
