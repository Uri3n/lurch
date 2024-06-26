//
// Created by diago on 4/18/2024.
//

#include <base.hpp>
#include <components.hpp>
#include <utility>


std::string
lurch::object::generate_id() {

    std::random_device              device;
    std::mt19937                    generator(device());
    std::uniform_int_distribution   distribution(0, 15);

    static const char*              character_set = "0123456789abcdef";
    std::string                     guid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

    for(auto& ch : guid) {
        if(ch == 'x')
            ch = character_set[distribution(generator)];

        else if(ch == 'y')
            ch = character_set[distribution(generator)];
    }

    return guid;
}


lurch::object::~object() {
    if(!id.empty()) {
        if(delete_from_database) {
            inst->db.delete_messages(id);
        }
        inst->log.write("freeing object from memory: " + id, log_type::INFO, log_noise::QUIET);
    } else {
        inst->log.write("an object with no GUID is being freed. Something is wrong. " + id, log_type::ERROR_MINOR, log_noise::QUIET);
    }
}


lurch::result<bool>
lurch::owner::create_child(const object_index index, const object_type type, const std::string& alias) {

    std::shared_ptr<object> child_ptr = inst->tree.create_object(index, std::nullopt, std::nullopt);
    if(child_ptr == nullptr) {
        return error("invalid object index.");
    }


    const auto store_res = inst->db.store_object(child_ptr->id, this->id, alias, type, index);
    if(!store_res) {
        return error(store_res.error());
    }


    inst->routing.send_ws_object_create_update(child_ptr->id, this->id, alias, type);
    children.emplace_back(child_ptr);


    inst->log.write(
        "A new object has been created with guid: " + child_ptr->id,
        log_type::SUCCESS,
        log_noise::NOISY
    );

    return { true };
}


lurch::result<bool>
lurch::owner::delete_child(const std::string &guid) {

    if(children.empty()) {
        return error("object has no children.");
    }

    for(auto it = children.begin(); it != children.end(); ++it) {
        auto child = *it;
        if(child->id == guid) {
            child->delete_from_database = true;
            children.erase(it);
            return { true };
        }
    }

    return error("child does not exist.");
}


lurch::result<bool>
lurch::owner::delete_all_children() {

    if(children.empty()) {
        return error("object has no children.");
    }

    for(auto& child : children) {
        child->delete_from_database = true;
    }

    children.clear();
    children.shrink_to_fit();
    return  { true };
}


lurch::owner::~owner() {

    if(delete_from_database) {
        for(auto& child : children) {
            child->delete_from_database = true;
        }

        if(const auto delete_result = inst->db.delete_object(id)) {
            inst->log.write(io::format_str("object with GUID {} has been deleted.", id), log_type::INFO, log_noise::NOISY);
        }

        inst->routing.send_ws_object_delete_update(id);
    }
}

lurch::leaf::~leaf() {

    if(delete_from_database) {
        if(const auto delete_result = inst->db.delete_object(id)) {
            inst->log.write(io::format_str("object with GUID {} has been deleted.", id), log_type::INFO, log_noise::NOISY);
        }

        inst->routing.send_ws_object_delete_update(id);
    }
}


lurch::owner::owner(std::optional<std::weak_ptr<owner>> parent, instance* inst) : object(inst), parent(std::move(parent)) {}
lurch::leaf::leaf(std::optional<std::weak_ptr<owner>> parent, instance *inst) : object(inst), parent(std::move(parent)) {}