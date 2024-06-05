//
// Created by diago on 2024-05-24.
//

#ifndef FUNCTION_PTRS_HPP
#define FUNCTION_PTRS_HPP
#include <Windows.h>
#include <winternl.h>

typedef HMODULE(WINAPI* fnLoadLibraryA)(
	LPCSTR lpLibFileName
);

typedef FARPROC(WINAPI* fnGetProcAddress)(
	HMODULE hModule,
	LPCSTR  lpProcName
);

typedef LPVOID(WINAPI* fnVirtualAlloc)(

	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD  flAllocationType,
	DWORD  flProtect
);

typedef BOOL(WINAPI* fnVirtualProtect)(

	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD  flNewProtect,
	PDWORD lpflOldProtect
);

typedef NTSTATUS(NTAPI* fnNtFlushInstructionCache)(

	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress,
	_In_ SIZE_T Length
);

typedef NTSTATUS(NTAPI* fnRtlConvertSidToUnicodeString)(

	PUNICODE_STRING UnicodeString,
	PSID            Sid,
	BOOLEAN         AllocateDestinationString
);

typedef NTSTATUS(NTAPI* fnNtQuerySystemInformation)(

	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID                    SystemInformation,
	ULONG                    SystemInformationLength,
	PULONG                   ReturnLength
);

typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(

	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength
);

typedef NTSTATUS(NTAPI* fnNtCreateThreadEx)(

	_Out_ PHANDLE ThreadHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ HANDLE ProcessHandle,
	_In_ void* StartRoutine,
	_In_opt_ PVOID Argument,
	_In_ ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
	_In_ SIZE_T ZeroBits,
	_In_ SIZE_T StackSize,
	_In_ SIZE_T MaximumStackSize,
	_In_opt_ void* AttributeList
);

typedef NTSTATUS(NTAPI* fnNtSetIoCompletion)(

	_In_ HANDLE IoCompletionHandle,
	_In_opt_ PVOID KeyContext,
	_In_opt_ PVOID ApcContext,
	_In_ NTSTATUS IoStatus,
	_In_ ULONG_PTR IoStatusInformation
);

typedef NTSTATUS(NTAPI* fnNtCreateSection)(

	_Out_ PHANDLE SectionHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PLARGE_INTEGER MaximumSize,
	_In_ ULONG SectionPageProtection,
	_In_ ULONG AllocationAttributes,
	_In_opt_ HANDLE FileHandle
);

typedef NTSTATUS(NTAPI* fnNtCreateProcessEx)(

	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ HANDLE ParentProcess,
	_In_ ULONG Flags, // PROCESS_CREATE_FLAGS_*
	_In_opt_ HANDLE SectionHandle,
	_In_opt_ HANDLE DebugPort,
	_In_opt_ HANDLE TokenHandle,
	_Reserved_ ULONG Reserved // JobMemberLevel
);

typedef NTSTATUS(NTAPI* fnRtlCreateProcessParametersEx)(

	_Out_ PRTL_USER_PROCESS_PARAMETERS* pProcessParameters,
	_In_ PUNICODE_STRING ImagePathName,
	_In_opt_ PUNICODE_STRING DllPath,
	_In_opt_ PUNICODE_STRING CurrentDirectory,
	_In_opt_ PUNICODE_STRING CommandLine,
	_In_opt_ PVOID Environment,
	_In_opt_ PUNICODE_STRING WindowTitle,
	_In_opt_ PUNICODE_STRING DesktopInfo,
	_In_opt_ PUNICODE_STRING ShellInfo,
	_In_opt_ PUNICODE_STRING RuntimeData,
	_In_ ULONG Flags // pass RTL_USER_PROC_PARAMS_NORMALIZED to keep parameters normalized
);


typedef enum _SECTION_INHERIT
{
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT;

typedef NTSTATUS(NTAPI* fnNtMapViewOfSection)(

	HANDLE          SectionHandle,
	HANDLE          ProcessHandle,
	PVOID* BaseAddress,
	ULONG_PTR       ZeroBits,
	SIZE_T          CommitSize,
	PLARGE_INTEGER  SectionOffset,
	PSIZE_T         ViewSize,
	SECTION_INHERIT InheritDisposition,
	ULONG           AllocationType,
	ULONG           Win32Protect
);

typedef bool(WINAPI* fnDllMain)(HINSTANCE, DWORD, LPVOID);


#endif //FUNCTION_PTRS_HPP
