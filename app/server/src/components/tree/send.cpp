//
// Created by diago on 2024-05-16.
//

#include <components.hpp>

lurch::search_ctx
lurch::instance::object_tree::send_message_r(
        const std::shared_ptr<object>& current,
        const std::string &guid,
        reciever_context& reciever_ctx
    ) {

    search_ctx curr_ctx;
    const auto owner_ptr    = dynamic_cast<owner*>(current.get());
    const auto leaf_ptr     = dynamic_cast<leaf*>(current.get());


    if(current->id == guid) {
        curr_ctx.keep_going = false;
        curr_ctx.obj_access = current->access;

        if(reciever_ctx.tok.access < current->access) {
            curr_ctx.response = error("invalid access level.");
        }

        if(owner_ptr) {
            curr_ctx.response = owner_ptr->receive(reciever_ctx);
        }

        if(leaf_ptr) {
            curr_ctx.response = leaf_ptr->receive(reciever_ctx);
        }

        curr_ctx.log_if_error   = reciever_ctx.log_if_error;
    }


    if(owner_ptr && curr_ctx.keep_going) {
        for(auto it = owner_ptr->children.begin(); it != owner_ptr->children.end(); ++it) {

            auto curr_child = *it;
            curr_ctx = send_message_r(
                curr_child,
                guid,
                reciever_ctx
            );

            if(curr_ctx.keep_going == false) {
                if(reciever_ctx.delete_self == true) {
                    reciever_ctx.delete_self = false;
                    curr_child->delete_from_database = true;
                    owner_ptr->children.erase(it);
                }
                break;
            }
        }
    }

    return curr_ctx;
}


std::pair<lurch::result<std::filesystem::path>, bool>
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

    std::pair<result<std::filesystem::path>, bool> result = { error("object not found."),  true };
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


lurch::search_ctx
lurch::instance::object_tree::send_message(const std::string& guid, const std::string& cmd_raw, reciever_context& reciever_ctx) {

    const std::shared_ptr<object> root_ptr = root;

    std::lock_guard<std::recursive_mutex> lock(tree_lock); //lock tree
    return send_message_r(
        root_ptr,
        guid,
        reciever_ctx
    );
}


lurch::result<std::filesystem::path>
lurch::instance::object_tree::upload_file(
        const std::string &guid,
        const std::string &file,
        const std::string &file_type,
        const access_level access
    ) {

    std::lock_guard<std::recursive_mutex> lock(tree_lock);
    const std::shared_ptr<object> root_ptr = root;

    return upload_file_r(root_ptr, guid, file, file_type, access).first;
}
