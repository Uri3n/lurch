//
// Created by diago on 2024-06-04.
//

#include <components.hpp>


lurch::result<lurch::config_data>
lurch::instance::init_config_data() {

    config_data config;

    if(std::filesystem::exists(LURCH_CONFIG_PATH)) {
        io::info("existing config.json found, loading data...");

        std::stringstream raw_filedata;
        const std::ifstream file(LURCH_CONFIG_PATH);
        crow::json::rvalue json;

        if(!file.is_open()) {
            return error("failed to open input stream.");
        }

        raw_filedata << file.rdbuf();
        json = crow::json::load(raw_filedata.str());

        if(!json) {
            return error("failed to load json");
        }

        if(        !json.has("bindaddr")
                || !json.has("port")
                || !json.has("use_https")
                || !json.has("cert_path")
                || !json.has("key_path")    ) {

            return error("invalid json keys. Corrupted?");
        }

        config.bindaddr     =   json["bindaddr"].s();
        config.port         =   static_cast<uint16_t>(std::stoul(json["port"].s()));
        config.use_https    =   json["use_https"].s() == "true";
        config.cert_path    =   json["cert_path"].s();
        config.key_path     =   json["key_path"].s();
    }

    else {
        io::info("no existing config.json found, one will be created.");
        config.bindaddr = io::prompt_for("enter server address: ");

        while(true) {
            try {
                config.port = 0;
                config.port = static_cast<uint16_t>(std::stoul(io::prompt_for("enter port: ")));
                break;
            }
            catch(const std::invalid_argument& e) {
                io::failure("that's not an integer.");
            }
            catch(const std::out_of_range& e) {
                io::failure("please stop being an idiot.");
            }
        }

        config.use_https = io::yesno("use HTTPS?");

        if(config.use_https) {
            if(io::yesno("generate new self-signed certificate?")) {

                if(!std::filesystem::exists("ssl/") && !std::filesystem::create_directory("ssl/")) {
                    return error("failed to create directory at \"ssl/\"");
                }

                config.cert_path = "ssl/cert.crt";
                config.key_path = "ssl/keyfile.key";

                if(!generate_self_signed_cert(config.cert_path, config.key_path, X509_VERSION_3)) {
                    return error("failed to generate certificate.");
                }
            }
            else {
                do {
                    config.cert_path = io::prompt_for("enter certificate path: ");
                } while(!std::filesystem::exists(config.cert_path));

                do {
                    config.key_path = io::prompt_for("enter key path: ");
                } while(!std::filesystem::exists(config.cert_path));

                config.key_path = std::filesystem::path(config.key_path).string();      // convert to platform independant path, removing any backslash if on Windows.
                config.cert_path = std::filesystem::path(config.cert_path).string();    // Otherwise, the JSON will not be formatted correctly (backslash must be double escaped).
            }
        }
        else {
            config.cert_path = "null";
            config.key_path = "null";
        }


        //
        // convert to json and write new config to disk
        //

        crow::json::wvalue json;
        json["bindaddr"] = config.bindaddr;
        json["port"] = std::to_string(config.port);
        json["use_https"] = config.use_https ? "true" : "false";
        json["cert_path"] = config.cert_path;
        json["key_path"] = config.key_path;

        std::ofstream file("config.json");
        if(!file.is_open()) {
            return error("could not open output file for config.");
        }

        file << json.dump();
        file.close();
    }

    return config;
}