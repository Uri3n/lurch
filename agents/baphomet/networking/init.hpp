//
// Created by diago on 2024-05-24.
//

#ifndef INIT_HPP
#define INIT_HPP
#include <Windows.h>
#include <winhttp.h>
#include <string>

bool open_session(_Out_ HINTERNET& hSession, _In_ const std::wstring& user_agent);
bool open_connection( _In_ const std::wstring& address,_In_ const INTERNET_PORT port,_In_ const HINTERNET hSession,_Out_ HINTERNET& hConnect);


#endif //INIT_HPP
