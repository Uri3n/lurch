//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

void
lurch::instance::object_tree::set_max_object_count(const uint32_t count) {
    this->max_object_count = count;
}

void
lurch::instance::object_tree::increment_object_count() {
    this->curr_object_count++;
}

std::shared_ptr<lurch::object>
lurch::instance::object_tree::create_object(object_index index, const std::optional<std::string> guid, const std::optional<std::weak_ptr<owner>> parent) {

    std::shared_ptr<object> obj = nullptr;

    switch(index) {
        case object_index::BAPHOMET:
            obj = std::make_shared<agent>(parent, this->inst);
            break;
        case object_index::GENERIC_GROUP:
            obj = std::make_shared<group>(parent, this->inst);
            break;
        case object_index::GENERIC_ROOT:
            obj = std::make_shared<lurch::root>(this->inst);
            break;
        default:
            break;
    }

    if(obj != nullptr) {
        obj->id = guid.value_or(object::generate_id());
    }

    return obj;
}


lurch::result<bool>
lurch::instance::object_tree::create_child(const std::string &parent_guid, object_index index) {
    increment_object_count();
    return error("unimplemented");
}