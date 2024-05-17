//
// Created by diago on 2024-05-16.
//

#include "../instance.hpp"


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