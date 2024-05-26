//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

//
//  lurch::instance::begin()
//  this function is responsible for:
//  - setting up routing, database, object tree.
//  - calling database::restore_objects to set up objects that were used last time.
//
// IMPORTANT:
//  if this function fails in any way, it will throw a fatal exception.
//  if we cannot set up the server instance, we cannot continue further.

void
lurch::instance::begin() {

    std::optional<std::string> initial_user = std::nullopt;
    std::optional<std::string> initial_password = std::nullopt;

    io::print_banner();
    io::info("Initializing Lurch server instance.");


    //
    // check for existing database
    //

    if(!std::filesystem::exists("db/lurch_database.db")) {
        io::info("Existing database not found, one will be created.");

        if(!std::filesystem::exists("db/") && !std::filesystem::create_directory("db/")) {
            throw std::runtime_error("failed to create database at directory \"/db\"");
        }

        initial_user = io::prompt_for("Please specify an initial username:");
        initial_password = io::prompt_for("Please specify an initial password:");
        std::cout << std::endl;

        if(initial_password.value().empty() || initial_user.value().empty()) {
            throw std::runtime_error("No default username or password provided.");
        }
    }


    //
    // initialize server config
    //

    const auto config = init_config_data();
    if(!config) {
        throw std::runtime_error(config.error_or("failed to initialize server config."));
    }


    tree.inst = this;
    routing.inst = this;

    const auto db_init = db.initialize(this, initial_user, initial_password);
    if(!db_init) {
        throw std::runtime_error(db_init.error());
    }

    const auto db_restore = db.restore_objects();
    if(!db_restore) {
        throw std::runtime_error(db_init.error());
    }

    db.delete_old_tokens();


    io::info(io::format_str( "\nAttempting bind to: {}:{}", config.value().bindaddr, std::to_string(config.value().port)));
    std::thread worker([&] {

        std::set_terminate(handle_uncaught_exception);
        if(config.value().use_https) {
            routing.run(
                config.value().bindaddr,
                config.value().port,
                config.value().cert_path,
                config.value().key_path
            );
        }
        else {
            routing.run(
                config.value().bindaddr,
                config.value().port,
                std::nullopt,
                std::nullopt
            );
        }

    });

    await_shutdown();
    worker.join();
}


//
// lurch::instance::generate_self_signed_cert()
// - generates self signed certificate and private key
// - writes key and cert to specified file paths, given as parameters.
// - cleans up resources generated by this disgusting C library
//

bool
lurch::instance::generate_self_signed_cert(const std::string &certfile_path, const std::string &keyfile_path, const long certificate_version) {

    if(certificate_version > 2 || certificate_version < 0) {
        return false;
    }

    EVP_PKEY* pkey = nullptr;
    X509* cert = nullptr;
    X509_NAME* name = nullptr;
    FILE* cert_fp = nullptr;
    FILE* key_fp = nullptr;
    errno_t err = 0x00;

    auto _ = lurch::defer([&] {
        if (cert_fp) fclose(cert_fp);
        if (key_fp) fclose(key_fp);
        if (name) X509_NAME_free(name);
        if (cert) X509_free(cert);
        if (pkey) EVP_PKEY_free(pkey);
    });


    io::info("\n\nbeginning certificate generation. version:" + std::to_string(certificate_version + 1));
    io::info("generating evp key...");

    pkey = EVP_PKEY_new();
    if (!pkey) {
        std::cerr << "error creating EVP_PKEY" << std::endl;
        return false;
    }

    io::info("creating evp key context...");
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if(!ctx) {
        std::cerr << "error creating EVP_PKEY_CTX" << std::endl;
        return false;
    }

    io::info("generating RSA key pair...");
    if (    EVP_PKEY_keygen_init(ctx) <= 0 ||
            EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, LURCH_RSA_KEYSIZE) <= 0 ||
            EVP_PKEY_keygen(ctx, &pkey) <= 0) {

        std::cerr << "failed to generate RSA key pair" << std::endl;
        return false;
    }

    io::info("generating x509 certificate...");
    EVP_PKEY_CTX_free(ctx);
    cert = X509_new();
    if(!cert) {
        std::cerr << "failed to generate X509 certificate." << std::endl;
        return false;
    }


    X509_set_version(cert, certificate_version);
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 12345);

    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);


    name = X509_NAME_new();
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"Self-Signed", -1, -1, 0);

    X509_set_subject_name(cert, name);
    X509_set_issuer_name(cert, name);
    X509_set_pubkey(cert, pkey);


    io::info("signing certificate...");
    if (!X509_sign(cert, pkey, EVP_sha256())) {
        std::cerr << "failed to sign certificate." << std::endl;
        return false;
    }

    io::info("writing certificate and key to disk...");
    err = fopen_s(&cert_fp, certfile_path.c_str(), "wb");
    if (err != 0 || !PEM_write_X509(cert_fp, cert)) {
        std::cerr << "failed to write certificate to file" << std::endl;
        return false;
    }

    err = fopen_s(&key_fp, keyfile_path.c_str(), "wb");
    if (err != 0 || !PEM_write_PrivateKey(key_fp, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        std::cerr << "failed to write private key to file" << std::endl;
        return false;
    }


    io::success("done.\n");
    return true;
}


//
// lurch::instance::init_config_data()
// - creates config.json if it does not exist
// - if it does exist, returns existing data
// - always returns config data (if no errors), regardless of whether the config file exists or not.
//

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


void
lurch::instance::post_message_interaction(
        const std::string& sender,
        const std::string& object,
        const std::optional<std::string>& response, //can be changed to just std::string
        const std::string& message_content,
        const access_level required_access
    ) {

    routing.send_ws_object_message_update(message_content, sender, object, required_access);
    db.store_message(object, sender, message_content);

    if(response && !response->empty()) {
        routing.send_ws_object_message_update(response.value(), object, object, required_access);
        db.store_message(object, object, response.value());
    }
}


void
lurch::instance::await_shutdown() {

    std::unique_lock<std::mutex> lock(mtx);
    io::info(io::format_str("thread with TID {}: awaiting shutdown condition.", std::this_thread::get_id()));
    shutdown_condition.wait(lock, [this]{ return shutdown; });

    routing.app.stop(); //can do additional cleanup tasks here...
}


void
lurch::instance::handle_uncaught_exception() {

    const std::exception_ptr exception = std::current_exception();
    std::cout << termcolor::red;

    try {
        std::rethrow_exception(exception);
    }
    catch(const std::exception& e) {
        std::cout << "[!] uncaught exception! what: " << e.what() << std::endl;
    }
    catch(...) {
        std::cout << "[!] unknown fatal exception!" << std::endl;
    }

    std::cout << termcolor::reset;

#ifdef LURCH_USE_STACKTRACE
    const std::stacktrace st = std::stacktrace::current();
    std::cout << "BEGIN STACKTRACE:" << std::endl;
    std::cout << st << std::endl;
#endif

    std::cout << "[!] exception routine finished. terminating..." << std::endl;
    std::exit(EXIT_FAILURE);
}
