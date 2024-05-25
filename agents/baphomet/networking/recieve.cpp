//
// Created by diago on 2024-05-24.
//

#include "recieve.hpp"
#include <iostream>

bool
recieve_file(
        _In_ const HINTERNET hConnect,
        _In_ const std::string &object_guid,
        _In_ const std::string &file_name,
        _In_ const std::string &session_token,
        _In_ const bool is_https,
        _Out_ std::string &file_buffer
    ) {

    DWORD bytes_read = 0;
    HINTERNET hRequest = nullptr;
    char* temp_buffer = nullptr;

    std::wstring auth_hdr = L"Authorization: Bearer " + std::wstring(session_token.begin(), session_token.end());
    std::wstring endpoint = L"/static/fileman/"
        + std::wstring(object_guid.begin(), object_guid.end()) + L'/'
        + std::wstring(file_name.begin(), file_name.end());

    auto _ = defer([&]() {
        if(hRequest != nullptr) {
            WinHttpCloseHandle(hRequest);
        }
        if(temp_buffer != nullptr) {
            HeapFree(GetProcessHeap(), 0, temp_buffer);
        }
    });

    if(!file_buffer.empty()) {
        file_buffer.clear();
    }

    hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        endpoint.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        (is_https ? WINHTTP_FLAG_SECURE : 0)
    );

    if(!hRequest) {
        return false;
    }


    if (!WinHttpAddRequestHeaders(
        hRequest,
        auth_hdr.c_str(),
        static_cast<DWORD>(auth_hdr.length()),
        WINHTTP_ADDREQ_FLAG_ADD
    )) {
        return false;
    }

    if (is_https) {
        DWORD flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                      SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                      SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                      SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

        WinHttpSetOption(
            hRequest,
            WINHTTP_OPTION_SECURITY_FLAGS,
            &flags,
            sizeof(DWORD)
        );
    }


    if (!WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0, 0, 0
    )) {
        return false;
    }

    if(!WinHttpReceiveResponse(hRequest, nullptr)) {
        return false;
    }


    uint32_t status_code = 0;
    uint32_t code_size = sizeof(status_code);

    if (!WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        nullptr,
        &status_code,
        (LPDWORD)&code_size,
        nullptr
    ) || status_code != 200) {
        return false;
    }

    if(status_code != 200) {
        return false;
    }


    temp_buffer = static_cast<char*>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        4096
    ));

    if(!temp_buffer) {
        return false;
    }


    while(WinHttpReadData(
        hRequest,
        temp_buffer,
        4096,
        &bytes_read
    ) && bytes_read > 0) {
        file_buffer.insert(file_buffer.end(), temp_buffer, temp_buffer + bytes_read);
        std::memset(temp_buffer, '\0', 4096);
    }

    return true;
}
