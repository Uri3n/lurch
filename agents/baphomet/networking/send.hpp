//
// Created by diago on 2024-05-24.
//

#ifndef SEND_HPP
#define SEND_HPP
#include <Windows.h>
#include <string>
#include <winhttp.h>
#include "../util/common.hpp"

namespace networking {

    bool
    send_object_message(
        _In_ const HINTERNET hConnect,
        _In_ const std::string& object_guid,
        _In_ const std::string& object_message,
        _In_ const std::string& session_token,
        _In_ const bool is_https,
        _Out_ std::string& response_body
    );

    bool
    upload_file(
            _In_ const HINTERNET hConnect,
            _In_ const std::string& object_guid,
            _In_ const HANDLE hFile,
            _In_ const std::string& session_token,
            _In_ const bool is_https
    );
}


#endif //SEND_HPP
