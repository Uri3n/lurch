//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"



std::pair<lurch::result<std::string>, bool>
lurch::instance::object_tree::send_message_r(
    const std::shared_ptr<object>& current,
    const std::string &guid,
    const command& cmd,
    const access_level access
    ) {


    const auto owner_ptr = dynamic_cast<owner*>(current.get());
    const auto leaf_ptr = dynamic_cast<leaf*>(current.get());

    if(current->id == guid) {

        if(access < current->access) {
            return { error("invalid access level"), false };
        }

        if(owner_ptr) {
            return { owner_ptr->recieve(cmd) , false };
        }

        if(leaf_ptr) {
            return { leaf_ptr->recieve(cmd), false };
        }

        return { error("invalid object type"), false };
    }

    std::pair<result<std::string>, bool> result = { error("object not found"), true };
    if(owner_ptr) {
        for(std::shared_ptr<object> child : owner_ptr->children) {
            result = send_message_r(
                child,
                guid,
                cmd,
                access
            );

            if(result.second == false) {
                break;
            }
        }
    }

    return result;
}


lurch::result<std::string>
lurch::instance::object_tree::send_message(const std::string& guid, const std::string& cmd_raw, const access_level access) {

    const result<command> cmd = argument_parser::parse(cmd_raw);
    const std::shared_ptr<object> root_ptr = root;

    std::lock_guard<std::recursive_mutex> lock(tree_lock); //lock tree
    return send_message_r(
        root_ptr,
        guid,
        cmd.value_or(command{.name = cmd_raw}),
        access
    ).first;
}


std::pair<lurch::result<bool>, bool>
lurch::instance::object_tree::upload_file_r(
        const std::shared_ptr<object>& current,
        const std::string &guid,
        const std::string &file,
        const std::string &file_type,
        const access_level access
    ) {

    const auto owner_ptr = dynamic_cast<owner*>(current.get());
    const auto leaf_ptr = dynamic_cast<leaf*>(current.get());

    if(current->id == guid) {

        if(access < current->access) {
            return { error("invalid access level"), false };
        }

        if(owner_ptr) {
            return { owner_ptr->upload(file, file_type), false };
        }

        if(leaf_ptr) {
            return { leaf_ptr->upload(file, file_type), false };
        }

        return { error("invalid object type"), false };
    }

    std::pair<result<bool>, bool> result = { "object not found.",  true };
    if(owner_ptr) {
        for(auto child : owner_ptr->children) {
            result = upload_file_r(
                child,
                guid,
                file,
                file_type,
                access
            );

            if(result.second == false) {
                break;
            }
        }
    }

    return result;
}


bool
lurch::instance::object_tree::upload_file(
        const std::string &guid,
        const std::string &file,
        const std::string &file_type,
        const access_level access
    ) {

    std::lock_guard<std::recursive_mutex> lock(tree_lock);
    const std::shared_ptr<object> root_ptr = root;

    const auto [fst, snd] = upload_file_r(root_ptr, guid, file, file_type, access);
    return fst.value_or(false);
}