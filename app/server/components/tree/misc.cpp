//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"
#include "../../objects/external/chatroom.hpp"


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

        case object_index::GENERIC_CHATROOM:
            obj = std::make_shared<chatroom>(parent, this->inst);
            obj->access = access_level::MEDIUM;
            break;

        default:
            break;
    }

    if(obj != nullptr) {
        obj->id = guid.value_or(object::generate_id());
    }

    return obj;
}


std::pair<lurch::result<lurch::access_level>, bool>
lurch::instance::object_tree::lookup_access_level_r(const std::shared_ptr<object> &current, const std::string &guid) {

    if(current->id == guid) {
        return { current->access, false };
    }

    std::pair<result<access_level>, bool> res = { error("object not found."), true };
    if(const auto owner_ptr = dynamic_cast<owner*>(current.get())) {
        for(auto child : owner_ptr->children) {
            res = lookup_access_level_r(child, guid);
            if(res.second == false) {
                break;
            }
        }
    }

    return res;
}


lurch::result<lurch::access_level>
lurch::instance::object_tree::lookup_access_level(std::string &guid) const {

    if(guid == "root") {
        guid = inst->db.query_root_guid().value_or(guid);
    }

    const auto root_ptr = root;
    return lookup_access_level_r(root_ptr, guid).first;
}