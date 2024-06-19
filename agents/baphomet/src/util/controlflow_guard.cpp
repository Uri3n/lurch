//
// Created by diago on 2024-06-16.
//

#include <misc.hpp>

bool
tasking::is_process_cfg_enforced() {

    fnNtQueryInformationProcess     pquery_process  = nullptr;
    EXTENDED_PROCESS_INFORMATION    proc_info       = { 0 };
    NTSTATUS                        status          = { 0 };

    proc_info.ExtendedProcessInfo                   = ProcessControlFlowGuardPolicy;
    proc_info.ExtendedProcessInfoBuffer             = 0;

    if(!get_function_ptr(pquery_process, "NTDLL.DLL", "NtQueryInformationProcess")) {
        return false;
    }

    const uint32_t flag = 16 | 36;
    status = pquery_process(
        GetCurrentProcess(),
        (PROCESSINFOCLASS)flag,
        &proc_info,
        sizeof(proc_info),
        nullptr
    );

    if(status != ERROR_SUCCESS) {
        DEBUG_PRINT("[!] Failed to check CFG enforcement! Status: 0x%0.8X\n", status);
    }

    DEBUG_PRINT("[*] CFG enforced process: %s\n", proc_info.ExtendedProcessInfoBuffer ? "True" : "False");
    return proc_info.ExtendedProcessInfoBuffer;
}


bool
tasking::add_cfg_call_target(char* module_base, void* function_address) {

    CFG_CALL_TARGET_INFO    cti         = { 0 };
    MEMORY_RANGE_ENTRY      range_entry = { 0 };
    VM_INFORMATION          vm_info     = { 0 };
    PIMAGE_NT_HEADERS       nt_hdrs     = nullptr;
    DWORD                   output      = 0;
    NTSTATUS                status      = 0;

    if(module_base == nullptr || function_address == nullptr) {
        return false;
    }


    nt_hdrs  = reinterpret_cast<decltype(nt_hdrs)>(module_base + ((PIMAGE_DOS_HEADER)module_base)->e_lfanew);
    range_entry.NumberOfBytes = (nt_hdrs->OptionalHeader.SizeOfImage + 0x1000 - 1) &~(0x1000 - 1);
    range_entry.VirtualAddress = module_base;

    cti.Flags  = CFG_CALL_TARGET_VALID;
    cti.Offset = PTR_TO_U64(function_address) - PTR_TO_U64(module_base);

    vm_info.dwNumberOfOffsets   = 1;
    vm_info.pdwOutput           = &output;
    vm_info.ptOffsets           = &cti;
    vm_info.MustBeZero          = nullptr;


    /*
    fnNtSetInformationVirtualMemory psetvm = nullptr;
    if(!get_function_ptr(psetvm, "NTDLL.DLL", "NtSetInformationVirtualMemory")) {
        return false;
    }

    status = psetvm(GetCurrentProcess(), VmCfgCallTargetInformation, 1, &range_entry, &vm_info, sizeof(vm_info));
    if(status != ERROR_SUCCESS) {
        DEBUG_PRINT("Error. Status: 0x%0.8X\n", status);
        return false;
    }
    */


    fnSetProcessValidCallTargets pset_call_targets = nullptr;
    if(!get_function_ptr(pset_call_targets, "KERNELBASE.DLL", "SetProcessValidCallTargets")) {
        DEBUG_PRINT("[!] failed to get SetProcessValidCallTargets\n");
        return false;
    }

    if(!pset_call_targets(GetCurrentProcess(), module_base, range_entry.NumberOfBytes, 1, &cti)) {
        DEBUG_PRINT("[!] SetProcessValidCallTargets failed. GLA: %lu\n", GetLastError());
        return false;
    }

    DEBUG_PRINT("[+] added valid CFG call target: 0x%p\n", function_address);
    return true;
}