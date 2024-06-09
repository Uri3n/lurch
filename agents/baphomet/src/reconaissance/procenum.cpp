//
// Created by diago on 2024-05-25.
//

#include <reconaissance.hpp>

std::string
recon::enumerate_processes() {

    std::string result;
    NTSTATUS status               = ERROR_SUCCESS;
    HANDLE   hprocess             = nullptr;

    uint32_t buffer_length        = 0;
    uint32_t total_proc_count     = 0;

    PSYSTEM_PROCESS_INFORMATION psystem_process_information  = nullptr;
    PSYSTEM_PROCESS_INFORMATION piterator                    = nullptr;
    fnNtQuerySystemInformation  pNtQuerySystemInfo           = nullptr;

    auto _  = defer([&]() {
        CLOSE_HANDLE(hprocess);
        FREE_HEAP_BUFFER(psystem_process_information);
    });


    pNtQuerySystemInfo = reinterpret_cast<fnNtQuerySystemInformation>\
        (GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), "NtQuerySystemInformation"));

    if(pNtQuerySystemInfo == nullptr) {
        return io::win32_failure("procenum", "GetProcAddress");
    }


    pNtQuerySystemInfo( //disregard the error code here, we just want the buffer size.
        SystemProcessInformation,
        nullptr,
        0,
        reinterpret_cast<PULONG>(&buffer_length)
    );

    buffer_length += 4096;
    psystem_process_information = static_cast<PSYSTEM_PROCESS_INFORMATION>(HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        buffer_length
    ));

    if(!psystem_process_information || !buffer_length) {
        return io::win32_failure("procenum", "HeapAlloc");
    }


    status = pNtQuerySystemInfo(
        SystemProcessInformation,
        psystem_process_information,
        buffer_length,
        reinterpret_cast<PULONG>(&buffer_length)
    );

    if(status != ERROR_SUCCESS) {
        return io::nt_failure("procenum", "NtQuerySystemInformation", status);
    }


    result += io::fmt_str("PID  ",  8);
    result += io::fmt_str("NAME  ", 47);
    result += io::fmt_str("ELEVATION  ", 17);
    result += "\n======  =============================================  ===============\n";

    piterator = psystem_process_information;
    while(true) {
        if(piterator->ImageName.Length) {
            result += io::fmt_str(std::to_string((uint32_t)piterator->UniqueProcessId), 8); //this is not a HANDLE. it's a PID. stupid microsoft

            size_t i = 0;
            std::string process_name;

            while(piterator->ImageName.Buffer[i] != 0) {
                if(piterator->ImageName.Buffer[i] >= 0 && piterator->ImageName.Buffer[i] < 128)
                    process_name += static_cast<char>(piterator->ImageName.Buffer[i]);
                else
                    process_name += '?';
                i++;
            }

            result += io::fmt_str(process_name, 47);

            bool elevation = false;
            if(!is_elevated((uint32_t)piterator->UniqueProcessId, elevation))
                result += io::fmt_str("access denied.", 17);
            else
                result += io::fmt_str(elevation ? "true" : "false", 17);

            result += '\n';
        }

        if(!piterator->NextEntryOffset) {
            break;
        }

        piterator = (PSYSTEM_PROCESS_INFORMATION)(reinterpret_cast<uint64_t>(piterator) + piterator->NextEntryOffset);
        ++total_proc_count;
    }

    result += "\ntotal running processes: " + std::to_string(total_proc_count);
    return result;
}



bool
recon::is_elevated(_In_ const uint32_t pid, _Out_ bool &elevation) {

    HANDLE hprocess         = nullptr;
    HANDLE htoken           = nullptr;
    uint32_t bytes_needed   = 0;
    TOKEN_ELEVATION buffer  = { 0 };

    auto _ = defer([&]() {
        CLOSE_HANDLE(hprocess);
        CLOSE_HANDLE(htoken);
    });


    hprocess = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        pid
    );

    if(hprocess == nullptr) {
        return false;
    }


    if(!OpenProcessToken(
        hprocess,
        TOKEN_QUERY,
        &htoken
    )) {
        return false;
    }

    if(!GetTokenInformation(
        htoken,
        TokenElevation,
        &buffer,
        sizeof(TOKEN_ELEVATION),
        reinterpret_cast<LPDWORD>(&bytes_needed)
    )) {
        return false;
    }

    elevation = (buffer.TokenIsElevated != 0);
    return true;
}
