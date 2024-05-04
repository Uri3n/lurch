//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

//
// Contains implementations for writing/changing data in the tree, and creating objects.
// as of right now the tree is controlled by a singular recursive mutex.
// functions should try on each lock to the mutex, and return right away if locking
// causes std::system_error to be thrown. Ideally the lock should only be used at most twice or three
// times by a single thread.
//

void
lurch::instance::object_tree::set_max_object_count(const uint32_t count) {
    this->max_object_count = count;
}

uint32_t
lurch::instance::object_tree::get_max_object_count() const {
    return this->max_object_count;
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


lurch::result<std::string>
lurch::instance::object_tree::send_message_r(const std::shared_ptr<object>& current, const std::string &guid, const command& cmd) {
    //This SHOULD be safe, because the ref count of "current" is being held by the loop variable in the previous call.
    const auto owner_ptr = dynamic_cast<owner*>(current.get());
    const auto leaf_ptr = dynamic_cast<leaf*>(current.get());

    if(current->id == guid) {
        if(owner_ptr) {
            return owner_ptr->recieve(cmd);
        }

        if(leaf_ptr) {
            return leaf_ptr->recieve(cmd);
        }

        return error("object is neither an owner or a leaf. Something is very wrong here.");
    }

    if(owner_ptr) {
        for(std::shared_ptr<object> child : owner_ptr->children) {
            if(result<std::string> object_found = send_message_r(child, guid, cmd)) {
                return object_found;
            }
        }
    }

    return error("object not found.");
}

lurch::result<std::string>
lurch::instance::object_tree::send_message(const std::string& guid, const std::string& cmd_raw) {

    result<command> cmd = argument_parser::parse(cmd_raw);
    const std::shared_ptr<object> root_ptr = root;
    std::lock_guard<std::recursive_mutex> lock(tree_lock); //lock tree

    if(cmd.has_value()) {
        return send_message_r(root_ptr, guid, cmd.value());
    }

    return error(cmd.error());
}


lurch::result<bool>
lurch::instance::object_tree::create_child(const std::string &parent_guid, object_index index) {
    increment_object_count();
    return error("unimplemented");
}
