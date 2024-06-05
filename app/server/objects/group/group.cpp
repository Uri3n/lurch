//
// Created by diago on 2024-05-01.
//

#include "group.hpp"
#include "../../components/instance.hpp"

lurch::result<std::filesystem::path>
lurch::group::upload(const std::string &file, const std::string &extension) {
    return error("dfdas");
}

lurch::result<std::string>
lurch::group::recieve(const lurch::command &cmd, bool& log_if_error) {
    inst->log.write("This is a test message", log_type::INFO, log_noise::NOISY);
    return OBJECT_EMPTY_RESPONSE;
}
