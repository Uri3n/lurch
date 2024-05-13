//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"


void
lurch::instance::object_tree::set_max_object_count(const uint32_t count) {
    max_object_count = count;
}

uint32_t
lurch::instance::object_tree::get_max_object_count() const {
    return max_object_count;
}

void
lurch::instance::object_tree::increment_object_count() {
    curr_object_count++;
}

std::shared_ptr<lurch::object>
lurch::instance::object_tree::create_object(const object_index index, const std::optional<std::string> guid, const std::optional<std::weak_ptr<owner>> parent) {

    std::shared_ptr<object> obj = nullptr;

    switch(index) {
        case object_index::BAPHOMET:
            obj = std::make_shared<baphomet>(parent, this->inst);
            obj->access = access_level::LOW;
            break;
        case object_index::GENERIC_GROUP:
            obj = std::make_shared<group>(parent, this->inst);
            obj->access = access_level::MEDIUM;
            break;
        case object_index::GENERIC_ROOT:
            obj = std::make_shared<lurch::root>(this->inst);
            obj->access = access_level::HIGH;
            break;
        default:
            break;
    }

    if(obj != nullptr) {
        obj->id = guid.value_or(object::generate_id());
    }

    return obj;
}


lurch::result<std::string>
lurch::instance::object_tree::send_message_r(
    const std::shared_ptr<object>& current,
    const std::string &guid,
    const command& cmd,
    const access_level access
    ) {

    // ref count of "current" is being held by the loop variable in the previous call.
    const auto owner_ptr = dynamic_cast<owner*>(current.get());
    const auto leaf_ptr = dynamic_cast<leaf*>(current.get());

    if(current->id == guid && access >= current->access) {

        if(owner_ptr) {
            return owner_ptr->recieve(cmd);
        }

        if(leaf_ptr) {
            return leaf_ptr->recieve(cmd);
        }
    }

    result<std::string> res = error("object not found");
    if(owner_ptr) {
        for(std::shared_ptr<object> child : owner_ptr->children) {
            res = send_message_r(child, guid, cmd, access);
            if(res.has_value()) {
                return res;
            }
        }
    }

    return res;
}


lurch::result<std::string>
lurch::instance::object_tree::send_message(const std::string& guid, const std::string& cmd_raw, const access_level access) {

    const result<command> cmd = argument_parser::parse(cmd_raw);
    const std::shared_ptr<object> root_ptr = root;
    std::lock_guard<std::recursive_mutex> lock(tree_lock); //lock tree

    if(cmd.has_value()) {
        return send_message_r(root_ptr, guid, cmd.value(), access);
    }

    return send_message_r(root_ptr, guid, command{ .name = cmd_raw }, access);
}

