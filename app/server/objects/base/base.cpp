//
// Created by diago on 4/18/2024.
//

#include "base.hpp"
#include "../../components/instance.hpp"
#include <utility>


std::string
lurch::object::generate_id() {

    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution distribution(0, 15);

    static const char* character_set = "0123456789abcdef";
    std::string guid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

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
        io::info("deleting object with GUID: " + id);
    } else {
        io::failure("an object with no GUID is being deleted. Something is wrong.");
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

    io::success("created child under " + this->id);
    return result<bool>(true);
}


lurch::result<bool>
lurch::owner::delete_child(const std::string &guid) {

    if(children.empty()) {
        return error("object has no children.");
    }

    for(auto it = children.begin(); it != children.end(); ++it) {
        if((*it)->id == guid) {
            children.erase(it);

            if(const auto delete_result = inst->db.delete_object(guid)) {
                io::success("deleted object " + guid);
            } else {
                io::failure(io::format_str("failed to delete {} from database.", guid));
                io::failure("error: " + delete_result.error());
            }

            inst->routing.send_ws_object_delete_update(guid);
            return result<bool>(true);
        }
    }

    return error("child does not exist.");
}


lurch::owner::owner(std::optional<std::weak_ptr<owner>> parent, instance* inst) : parent(std::move(parent)), inst(inst) {}
lurch::leaf::leaf(std::optional<std::weak_ptr<owner>> parent, instance *inst) : parent(std::move(parent)), inst(inst) {}