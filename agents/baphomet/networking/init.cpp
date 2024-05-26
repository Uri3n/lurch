//
// Created by diago on 2024-05-24.
//

#include "init.hpp"

bool
networking::open_session(
        _Out_ HINTERNET& hSession,
        _In_ const std::wstring& user_agent
) {

    hSession = WinHttpOpen(
        user_agent.c_str(),
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    return hSession != nullptr;
}


bool
networking::open_connection(
        _In_ const std::wstring& address,
        _In_ const INTERNET_PORT port,
        _In_ const HINTERNET hSession,
        _Out_ HINTERNET& hConnect
    ) {

    hConnect = WinHttpConnect(
        hSession,
        address.c_str(),
        port,
        0
    );

    return hConnect != nullptr;
}