//
// Created by diago on 2024-05-24.
//

#ifndef COMMON_HPP
#define COMMON_HPP
#include <string>
#include <winternl.h>

#define CLOSE_HANDLE(HANDLE) if(HANDLE != nullptr && HANDLE != INVALID_HANDLE_VALUE){CloseHandle(HANDLE);}
#define FREE_HEAP_BUFFER(BUFFER) if(BUFFER != nullptr){HeapFree(GetProcessHeap(), 0, BUFFER);}

#define SIZE_OF_PAGE     0x1000
#define COMMAND_DELIMITER '!'

#define PAGE_ALIGN(x)    (((uint64_t)x) + ((SIZE_OF_PAGE - (((uint64_t)x) & (SIZE_OF_PAGE - 1))) % SIZE_OF_PAGE))
#define PTR_TO_U64(ptr)  reinterpret_cast<uint64_t>(ptr)
#define INT_TO_U64(x)    static_cast<uint64_t>(x)

#define PS_REQUEST_BREAKAWAY                    1
#define PS_NO_DEBUG_INHERIT                     2
#define PS_INHERIT_HANDLES                      4
#define PS_LARGE_PAGES                          8
#define PS_ALL_FLAGS                            (PS_REQUEST_BREAKAWAY | PS_NO_DEBUG_INHERIT  | PS_INHERIT_HANDLES   | PS_LARGE_PAGES)
#define IO_COMPLETION_ALL_ACCESS                (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)
#define RTL_USER_PROC_PARAMS_NORMALIZED			0x00000001

#define NtCurrentProcess() ( ( HANDLE ) ( LONG_PTR ) -1 )

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
((ULONG_PTR)(ntheader) +                                                \
FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                      \
((ntheader))->FileHeader.SizeOfOptionalHeader                           \
))

template<typename T>
class defer_wrapper {
    T callable;
public:

    auto call() -> decltype(callable()) {
        return callable();
    }

    explicit defer_wrapper(T func) : callable(func) {}
    ~defer_wrapper() { callable(); }
};

template<typename T>
defer_wrapper<T> defer(T callable) {
    return defer_wrapper<T>(callable);
}

#endif //COMMON_HPP
