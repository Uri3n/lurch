//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"


lurch::result<std::string>
lurch::instance::router::hdr_extract_token(const crow::request &req) {
    try {
        std::string token = req.get_header_value("Authorization").substr(7);
        if(token.empty()) {
            throw std::exception();
        }

        return { token };
    }
    catch(...) {
        return error("Invalid token.");
    }
}


std::string
lurch::instance::router::file_template(
        const std::string& uri_path,
        const std::string& base_name,
        const std::string& extension
    ) {


    //
    // can be altered later if we need more file formats
    //

    static constexpr filetype_pair<2> audio_formats = {
        {".mp3",".wav"},
        R"(<audio controls title="{}">
            <source src="{}" type="audio/mpeg">
                Your browser does not support the audio element.
            </audio>)"
    };

    static constexpr filetype_pair<5> image_formats = {
        {".png",".jpg",".jpeg",".bmp",".gif"},
        R"(<img src="{}" class="placeholder-image" title="{}">)"
    };

    static constexpr filetype_pair<1> video_formats = {
        {".mp4"},
        R"(<video controls title="{}">
            <source src="{}" type="video/mp4"></source>
                Your browser does not support video elements.
        </video>)"
    };

    static constexpr std::string_view generic_file_template =
        R"(<a href="{}" download>
                    <div class="file has-name is-boxed">
                        <label class="file-label">
                            <span class="file-cta">
                              <span class="file-icon">
                              </span>
                              <span class="file-label"> Downloadable File </span>
                            </span>
                            <span class="file-name"> {} </span>
                          </label>
                    </div>
                </a>)";

    for(const auto& ext : image_formats.extensions) {
        if(ext == extension) {
            return io::format_str(image_formats.html, uri_path, base_name);
        }
    }

    for(const auto& ext : audio_formats.extensions) {
        if(ext == extension) {
            return io::format_str(audio_formats.html, base_name, uri_path);
        }
    }

    for(const auto& ext : video_formats.extensions) {
        if(ext == extension) {
            return io::format_str(video_formats.html, base_name, uri_path);
        }
    }

    return io::format_str(generic_file_template, uri_path, base_name);
}


bool
lurch::instance::router::verify_token(const crow::request &req, const access_level required_access) const {

    if(const auto result = hdr_extract_token(req)) {
        if(inst->db.match_token(result.value(), required_access)) {
            io::success("authenticated token " + result.value());
            return true;
        }
    }

    return false;
}


lurch::result<std::pair<std::string, std::string>>
lurch::instance::router::hdr_extract_credentials(const crow::request &req) {

    try {
        std::string encoded_creds = req.get_header_value("Authorization").substr(6);
        if(encoded_creds.empty()) {
            throw std::exception();
        }

        std::string decoded_creds  = crow::utility::base64decode(encoded_creds, encoded_creds.size());

        size_t found = decoded_creds.find(':');
        if(found == std::string::npos) {
            throw std::exception();
        }

        return std::make_pair(decoded_creds.substr(0, found), decoded_creds.substr(found+1));
    }
    catch(...) {
        return error("Invalid credentials provided.");
    }
}