//
// Created by diago on 2024-04-25.
//

#include "root.hpp"
#include "../../components/instance.hpp"

std::string
lurch::root::recieve(const command &cmd) {

    if(cmd.name == "create") {
        if(const auto create_res = create_child(object_index::GENERIC_GROUP, object_type::GROUP, "groupazz")) {
            return "child creation successful.";
        } else {
            return create_res.error();
        }
    }

    else {
        if(const auto del_res = delete_child(cmd.name)) {
            return "deleted successfully";
        } else {
            return del_res.error();
        }
    }
}
