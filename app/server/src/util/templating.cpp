//
// Created by diago on 2024-06-21.
//

#include <templating.hpp>


std::string
lurch::templates::command_list(const std::string& header, const std::vector<std::pair<std::string, std::string>>& commands) {

    auto command_container = crow::mustache::compile(
        R"(<div class="content">
            <h1>{{header}}</h1>
            {{{content}}}
          </div>)"
    );

    auto command = crow::mustache::compile(
        R"(<h3>{{title}}</h3>
            <div class="message is-dark is-small">{{{description}}}</div>)"
    );


    std::string buffer;
    for(const auto &[title, description] : commands) {
        crow::mustache::context ctx;
        ctx["title"]        = title;
        ctx["description"]  = description;

        buffer += command.render_string(ctx);
    }


    crow::mustache::context final_ctx;
    final_ctx["header"]  = header;
    final_ctx["content"] = buffer;

    return command_container.render_string(final_ctx);
}


std::string
lurch::templates::flag_list(const std::string &header, const std::vector<flag_descriptor>& flag_descriptors) {

    auto flag_list_container = crow::mustache::compile(
        R"(<div class="content">
            <h1>{{header}}</h1>
            {{{content}}}
          </div>)"
    );

    auto flag_content = crow::mustache::compile(
        R"(<h3>{{name}}</h3>
           <h5>{{type}}</h5>
           <h5>{{required}}</h5>
           <div class="message is-dark is-small">{{{description}}}</div>)"
    );


    std::string buffer;
    for(const auto &[name, type, required, description] : flag_descriptors) {
        crow::mustache::context ctx;
        ctx["name"]        = name;
        ctx["type"]        = type;
        ctx["required"]    = required;
        ctx["description"] = description;

        buffer += flag_content.render_string(ctx);
    }

    crow::mustache::context final_ctx;
    final_ctx["header"]  = header;
    final_ctx["content"] = buffer;

    return flag_list_container.render_string(final_ctx);
}


std::string
lurch::templates::terminal_media(const std::string& uri_path, const std::string& base_name, const std::string& extension) {

    crow::mustache::context ctx;
    ctx["title"]  = base_name;
    ctx["source"] = uri_path;


    if(extension == ".mp3" || extension == ".wav") {
        return crow::mustache::compile(
            R"(<audio controls title="{{title}}" draggable="false" class="terminal-media-element">
                <source src="{{{source}}}" type="audio/mpeg">
                    Your browser does not support the audio element.
                </audio>)"
        )
        .render_string(ctx);
    }

    if(extension == ".mp4") {
        return crow::mustache::compile(
            R"(<video controls title="{{title}}" draggable="false" class="terminal-media-element">
                <source src="{{{source}}}" type="video/mp4"></source>
                    Your browser does not support video elements.
            </video>)"
        )
        .render_string(ctx);
    }

    if(extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".gif") {
        return crow::mustache::compile(
            R"(<img src="{{{source}}}" draggable="false" class="terminal-media-element" title="{{title}}">)"
        )
        .render_string(ctx);
    }

    return crow::mustache::compile(
        R"(<a href="{{{source}}}" download draggable="false" style="padding-bottom:1.5vh">
                <div class="file has-name is-boxed">
                    <label class="file-label">
                        <span class="file-cta">
                          <span class="file-icon">
                          </span>
                          <span class="file-label"> Downloadable File </span>
                        </span>
                        <span class="file-name"> {{title}} </span>
                      </label>
                </div>
            </a>)"
    )
    .render_string(ctx);
}

