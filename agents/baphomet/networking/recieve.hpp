//
// Created by diago on 2024-05-24.
//

#ifndef RECIEVE_HPP
#define RECIEVE_HPP
#include <windows.h>
#include <winhttp.h>
#include <string>
#include "../util/common.hpp"

namespace networking {
    bool recieve_file(
        _In_ const HINTERNET hConnect,
        _In_ const std::string& object_guid,
        _In_ const std::string& file_name,
        _In_ const std::string& session_token,
        _In_ const bool is_https,
        _Out_ std::string& file_buffer
    );
}

#endif //RECIEVE_HPP
