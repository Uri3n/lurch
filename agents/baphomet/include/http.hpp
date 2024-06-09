//
// Created by diago on 2024-06-04.
//

#ifndef HTTP_HPP
#define HTTP_HPP
#include <Windows.h>
#include <winhttp.h>
#include <string>
#include <common.hpp>

namespace networking {
    namespace http {
        bool open_session(_Out_ HINTERNET& hSession, _In_ const std::wstring& user_agent);
        bool open_connection( _In_ const std::wstring& address,_In_ const INTERNET_PORT port,_In_ const HINTERNET hSession,_Out_ HINTERNET& hConnect);

        bool
        recieve_file(
            _In_ HINTERNET hConnect,
            _In_ const std::string& object_guid,
            _In_ const std::string& file_name,
            _In_ const std::string& session_token,
            _In_ const bool is_https,
            _Out_ std::string& file_buffer
        );

        bool
        send_object_message(
            _In_ HINTERNET hConnect,
            _In_ const std::string& object_guid,
            _In_ const std::string& object_message,
            _In_ const std::string& session_token,
            _In_ const bool is_https,
            _Out_ std::string& response_body
        );

        bool
        upload_file(
                _In_ HINTERNET hConnect,
                _In_ const std::string& object_guid,
                _In_ const HANDLE hFile,
                _In_ const std::string& session_token,
                _In_ const bool is_https
        );

    }
}

#endif //HTTP_HPP
