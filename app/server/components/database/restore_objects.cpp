//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"


void
lurch::instance::database::restore_objects_r(std::shared_ptr<owner> object, size_t& total_restored) {

    io::info("retrieving children of: " + object->id);
    result<array_of_children> children = query_object_children(object->id);
    if(!children) {
        io::info("no children.");
        return;
    }

    io::info(std::string("number of children: ") + std::to_string(children.value().size()) );
    for(const auto &[guid, alias, type, index] : children.value()) {
        std::shared_ptr<lurch::object> child_ptr = inst->tree.create_object(
            index,
            guid,
            std::nullopt //Parent pointers are unused for now. May change this later.
        );

        io::info(io::format_str("restoring object {} :: {}", guid, alias));
        ++total_restored;

        if(const auto owner_ptr = std::dynamic_pointer_cast<owner>(object->children.emplace_back(std::move(child_ptr)))) {
            io::info("owner. Restoring children.");
            restore_objects_r(owner_ptr,total_restored);
        } else {
            io::info("leaf.");
        }
    }
}


lurch::result<bool>
lurch::instance::database::restore_objects() {

    result<std::string> root_guid = query_root_guid();
    size_t total_restored_objects = 0;

    if(!root_guid) {
        root_guid = result<std::string>(object::generate_id());
        auto res = store_object(
            root_guid.value(),
            std::nullopt,
            "root",                             //alias
            object_type::ROOT,
            object_index::GENERIC_ROOT          //can be changed
        );

        if(!res) {
            return error(res.error());          //propagate error, will throw after.
        }

    } else {
        ++total_restored_objects;
        io::success("restored root object: " + root_guid.value());
    }

    std::shared_ptr<object> root_ptr = inst->tree.create_object(
        object_index::GENERIC_ROOT,
        root_guid.value(),
        std::nullopt
    );

    inst->tree.root = std::move(root_ptr);
    inst->tree.increment_object_count();

    restore_objects_r(std::dynamic_pointer_cast<owner>(inst->tree.root), total_restored_objects);

    io::success("finished restoration.");
    io::success("objects restored: " + std::to_string(total_restored_objects));

    return { true };
}