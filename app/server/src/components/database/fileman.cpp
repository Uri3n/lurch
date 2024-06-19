//
// Created by diago on 2024-05-16.
//

#include <components.hpp>

namespace fs = std::filesystem;


lurch::result<std::filesystem::path>
lurch::instance::database::fileman_create(const std::string_view& raw_contents, const std::string &extension, const std::string &guid, const bool is_binary) {

    std::lock_guard<std::mutex> lock(fileman_mtx);

    if(extension.find('.') != std::string::npos) {
        return error("do not include '.' characters inside of the file extension.");
    }

    if(!fs::exists("static/fileman/" + guid)) {
        fs::create_directory("static/fileman/" + guid);
    }

    const fs::path unique = io::format_str("static/fileman/{}/{}.{}",
        guid,
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()),
        extension
    );

    if(exists(unique)) {
        return error("already exists."); //this will literally never happen. But who knows
    }

    try {
        auto file = (is_binary ? std::ofstream(unique, std::ios::binary) : std::ofstream(unique));
        if(!file.is_open()) {
            throw std::runtime_error("could not open output file.");
        }

        file.write(raw_contents.data(), static_cast<std::streamsize>(raw_contents.size()));
        file.close();

        return { unique };
    }
    catch(const std::exception& e) {
        return error(std::string("exception: ") + e.what());
    }
    catch(...) {
        return error("unknown uncaught exception.");
    }
}


lurch::result<std::filesystem::path>
lurch::instance::database::fileman_get_by_extension(const std::string &guid, const std::string &extension) {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    const std::string           path = "static/fileman/" + guid;

    if(!fs::exists(path) || !fs::is_directory(path)) {
        return error("No files exist.");
    }

    for(const auto& file : fs::directory_iterator(path)) {
        if(file.path().extension() == extension) {
            return { file.path() };
        }
    }

    return error("File does not exist.");
}


lurch::result<std::stringstream>
lurch::instance::database::fileman_get_raw(const std::string &name, const std::string &guid) {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    const std::string filename = io::format_str("static/fileman/{}/{}", guid, name);

    if(!fs::exists(filename) || !fs::is_regular_file(filename)) {
        return error("file does not exist, or is a directory.");
    }

    const std::ifstream stream(filename);
    if(!stream.is_open()) {
        return error("failed to open file");
    }

    std::stringstream buff;
    buff << stream.rdbuf();

    return buff;
}


void
lurch::instance::database::fileman_wipe(const std::string &guid) {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    if(fs::exists("static/fileman/" + guid)) {
        fs::remove_all("static/fileman/" + guid);
    }
}


void
lurch::instance::database::fileman_wipe_all() {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    if(!fs::exists("static/fileman/") || !fs::is_directory("static/fileman")) {
        return;
    }

    size_t total_deleted = 0;
    for(const auto& entry : std::filesystem::directory_iterator("static/fileman")) {
        if(fs::is_directory(entry)) {
            try {
                fs::remove_all(entry);
                ++total_deleted;
            }
            catch(const std::exception& e) {
                inst->log.write(
                    io::format_str("couldn't erase directory {}, error: {}", entry.path().string(), e.what()),
                    log_type::ERROR_MINOR,
                    log_noise::REGULAR
                );
            }
        }
    }

    inst->log.write("fileman_wipe_all(): directories erased: " + std::to_string(total_deleted), log_type::SUCCESS, log_noise::REGULAR);
}


lurch::result<std::vector<std::filesystem::path>>
lurch::instance::database::fileman_get_file_list(const std::string &guid) {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    const std::string path = "static/fileman/" + guid;
    std::vector<fs::path> paths;

    if(!fs::exists(path) || !fs::is_directory(path)) {
        return error("No files exist.");
    }

    for(const auto& entry : fs::directory_iterator(path)) {
        paths.emplace_back(entry.path().filename());
    }

    if(paths.empty()) {
       return error("no files in this directory.");
    }

    return paths;
}


bool
lurch::instance::database::fileman_delete_file(const std::string &name, const std::string &guid) {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    if(!fs::exists("static/fileman/" + guid) || !fs::is_directory("static/fileman/" + guid)) {
        return false;
    }

    for(const auto& entry : fs::directory_iterator(fs::path("static/fileman/" + guid)) ) {
        if(entry.path().filename() == name) {
            fs::remove_all(entry);
            return true;
        }
    }

    return false;
}


bool
lurch::instance::database::fileman_delete_all_files(const std::string &guid) {

    std::lock_guard<std::mutex> lock(fileman_mtx);
    if(!fs::exists("static/fileman/" + guid) || !fs::is_directory("static/fileman/" + guid)) {
        return false;
    }

    for(const auto& entry : fs::directory_iterator(fs::path("static/fileman/" + guid))) {
        fs::remove_all(entry);
    }

    return true;
}

