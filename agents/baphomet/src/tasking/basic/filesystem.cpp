//
// Created by diago on 2024-05-25.
//

#include <filesystem.hpp>

//
// Generic commands involving the filesystem, including:
// - ls
// - cd
// - pwd
// - rm
// - mkdir
//

std::string
tasking::pwd() {

    char curr_directory[MAX_PATH] = { 0 };
    if(!GetCurrentDirectoryA(MAX_PATH, curr_directory)) {
        return io::win32_failure("pwd", "GetCurrentDirectoryA");
    }

    return { curr_directory} ;
}


std::string
tasking::ls() {

    char curr_dir[MAX_PATH] = { 0 };
    WIN32_FIND_DATAA    file_data = { 0 };
    HANDLE              hFind     = nullptr;

    std::string         search_pattern;
    std::string         result;

    if(!GetCurrentDirectoryA(MAX_PATH, curr_dir)) {
        return io::win32_failure("ls", "GetCurrentDirectoryA");
    }

    search_pattern = std::string(curr_dir) + "\\*";
    hFind = FindFirstFileA(search_pattern.c_str(), &file_data);
    if(hFind == INVALID_HANDLE_VALUE) {
        return io::win32_failure("ls", "FindFirstFileA");
    }

    do {
        result += file_data.cFileName;
        result += '\n';
    } while(FindNextFileA(hFind, &file_data));

    if(result.empty()) {
        result = "The current directory is empty.";
    }

    FindClose(hFind);
    return result;
}


std::string
tasking::cd(const std::string& directory) {

    char new_directory[MAX_PATH] = { 0 };
    if(!SetCurrentDirectoryA(directory.c_str())) {
        return io::win32_failure("cd", "SetCurrentDirectoryA");
    }

    if(!GetCurrentDirectoryA(MAX_PATH, new_directory)) {
        return "Changed directories.";
    }

    return std::string("Changed current directory to: ") + new_directory;
}


std::string
tasking::cat(const std::string &file_name) {

    std::string file_contents;
    HANDLE hFile = nullptr;
    uint32_t file_size = 0;
    uint32_t bytes_read = 0;


    hFile = CreateFileA(
        file_name.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        if(GetLastError() == ERROR_FILE_NOT_FOUND) {
            return "file does not exist.";
        }
        return io::win32_failure("cat", "CreateFileA");
    }


    file_size = GetFileSize(hFile, nullptr);
    if(file_size == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return io::win32_failure("cat", "GetFileSize");
    }

    file_contents.resize(file_size);
    if(!ReadFile(
        hFile,
        &file_contents[0],
        file_size,
        reinterpret_cast<LPDWORD>(&bytes_read),
        nullptr
    ) || bytes_read != file_size) {
        CloseHandle(hFile);
        return io::win32_failure("cat", "ReadFile");
    }


    CloseHandle(hFile);
    return file_contents;
}


std::string
tasking::mkdir(const std::string &directory_name) {

    if(directory_name.empty()) {
        return io::failure("mkdir", "empty directory name");
    }

    if(!CreateDirectoryA(directory_name.c_str(), nullptr)) {
        return io::win32_failure("mkdir", "CreateDirectoryA");
    }

    return "successfully created new directory " + directory_name;
}


std::string
tasking::cp(const std::string &source, const std::string &destination) {

    if(source.empty() || destination.empty()) {
        return io::failure("cp", "invalid arguments");
    }

    if(!CopyFileA(
        source.c_str(),
        destination.c_str(),
        TRUE
    )) {
        return io::win32_failure("cp", "CopyFileA");
    }

    return "successfully copied:\n" + source + " --> " + destination;
}


std::string
tasking::rm(const std::string &directory_entry) {

    const bool is_directory = (directory_entry.find_first_of('.') == std::string::npos);

    if(is_directory && !RemoveDirectoryA(directory_entry.c_str())) {
        return io::win32_failure("rm", "RemoveDirectoryA");
    }

    if(!is_directory && !DeleteFileA(directory_entry.c_str())) {
        return io::win32_failure("rm", "DeleteFileA");
    }

    return "deleted directory entry successfully: " + directory_entry;
}


HANDLE
tasking::get_file_handle(const std::string &directory_entry) {

    HANDLE hFile = CreateFileA(
        directory_entry.c_str(),
        FILE_GENERIC_READ | FILE_GENERIC_WRITE | DELETE | SYNCHRONIZE,
        FILE_SHARE_READ   | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(hFile == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    return hFile;
}










