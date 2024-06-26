//
// Created by diago on 2024-06-24.
//

#include <group.hpp>
#include <components.hpp>

lurch::result<std::string>
lurch::group::members(reciever_context &ctx) const {

    if(const auto children = inst->db.query_object_children(id)) {

        std::string buffer;
        buffer += io::format_str("{:<37} {:<30} {:<8} {:<6}", "GUID", "Alias", "Type", "Index") + '\n';
        buffer += io::format_str("{:=<37} {:=<30} {:=<8} {:=<6}", "=", "=", "=", "=") + '\n';

        for(const auto &[guid, alias, type, index] : *children) {
            buffer += io::format_str("{:<37} {:<30} {:<8} {:<6}", guid, alias, io::type_to_str(type), std::to_string(static_cast<int64_t>(index))) + '\n';
        }

        if(buffer.back() == '\n') {
            buffer.pop_back();
        }

        return { buffer };
    }
    else {
        return error(children.error());
    }
}


lurch::result<std::string>
lurch::group::add_member(reciever_context &ctx) {

    const auto [name] = ctx.cmd.get<std::string>("--name", "-n").done();

    if(*name == "baphomet" || *name == "Baphomet") {
        return create_child(object_index::BAPHOMET, object_type::AGENT, "Baphomet")
            .and_then([&](bool _) {
                return result<std::string>("Successfully created member.");
            })
            .or_else([&](std::string err) {
                return result<std::string>(error(err));
            });
    }

    return error("Invalid name provided."); // We only support "Baphomet" as a name currently
}


lurch::result<std::string>
lurch::group::disband(reciever_context &ctx) {
    ctx.delete_self = true;
    return OBJECT_EMPTY_RESPONSE;
}



lurch::result<std::string>
lurch::group::issue(reciever_context& ctx) {

    const auto [ message ] = ctx.cmd.get<std::string>("--message", "-m").done();

    std::vector<std::pair<std::string, std::string>> responses; //id, response
    reciever_context new_ctx;

    new_ctx.address     = ctx.address;
    new_ctx.tok         = ctx.tok;
    new_ctx.message_raw = *message;
    new_ctx.cmd         = argument_parser::parse(*message).value_or(command{.name = new_ctx.message_raw});


    //
    // Send message to children
    //

    for(auto it = children.begin(); it != children.end(); ) {

        result<std::string> child_res;

        if(auto* owner_ptr = dynamic_cast<owner*>(it->get())) {
            child_res = owner_ptr->receive(new_ctx);
        } else if(auto* leaf_ptr = dynamic_cast<leaf*>(it->get())) {
            child_res = leaf_ptr->receive(new_ctx);
        } else {
            return error("Unspecified error.");
        }

        responses.emplace_back(std::make_pair((*it)->id, child_res.value_or(child_res.error())));

        if(new_ctx.delete_self) {
            new_ctx.delete_self = false;
            it = children.erase(it);
        } else {
            ++it;
        }
    }


    //
    // Generate response
    //

    std::string buffer;
    for(const auto&[id, response] : responses) {
        buffer += io::format_str("OBJECT: {}\n{}\n\n", id, response);
    }

    if(buffer.back() == '\n') {
        buffer.pop_back();
    }

    return buffer;
}


lurch::result<std::string>
lurch::group::remove_member(reciever_context &ctx) {

    const auto [guid, all] =
        ctx.cmd.get<std::string>("--guid", "-g")
            .with<empty>("--all", "-a")
            .done();


    if((guid && all) || (!guid && !all)) {
        return error("Invalid flags. Please provide either a GUID or specify --all.");
    }


    if(guid) {
        return delete_child(*guid)
            .and_then([&](bool _) {
                return result<std::string>("Successfully removed member.");
            })
            .or_else([&](std::string err) {
                return result<std::string>(error(err));
            });
    }


    // --all
    return delete_all_children()
        .and_then([&](bool _ ) {
            return result<std::string>("Successfully removed all members.");
        })
        .or_else([&](std::string err) {
            return result<std::string>(error(err));
        });
}


lurch::result<std::string>
lurch::group::groupfiles(reciever_context& ctx) const {

    const auto [file_name] = ctx.cmd.get<std::string>("--get", "-g").done();
    std::string buffer;


    for(const auto& child : children) {                                        // check through each child
        if(const auto file_list = inst->db.fileman_get_file_list(child->id)) { // file list for each child
            for(const auto& file_path : *file_list) {                          // each path in that list

                if(file_name) {
                    if(buffer.empty() && file_path.string() == *file_name) {
                        buffer += templates::terminal_media(
                            io::format_str("static/fileman/{}/{}", child->id, file_path.string()),
                            file_path.filename().string(),
                            file_path.extension().string()
                        );
                    }
                } else {
                    buffer += file_path.string() + '\n';
                }

            }
        }
    }


    if(buffer.empty() && file_name) {
        return error("That file does not exist.");
    }

    if(!file_name) {
        if(buffer.empty()) {
            return error("No members currently own files.");
        }
        if(buffer.back() == '\n') {
            buffer.pop_back();
        }
    }

    return buffer;
}