//
// Created by diago on 2024-05-27.
//

#include <injection.hpp>

bool
tasking::initialize_dll_info(const std::string &dll_buffer, dll_info *pdll_info) {

    //
    // C++ casts do not work for some of these, for instance one of the casts below,
    // const char* -> uint8_t*. Need to use C-style casts instead.
    //

	if(((PIMAGE_DOS_HEADER)dll_buffer.data())->e_magic != IMAGE_DOS_SIGNATURE) {
		return false;
	}

    pdll_info->pFileBuffer  =    (uint8_t*)dll_buffer.data();
    pdll_info->pImgNtHdrs   =    (PIMAGE_NT_HEADERS)(dll_buffer.data() + ((PIMAGE_DOS_HEADER)dll_buffer.data())->e_lfanew);
    if (pdll_info->pImgNtHdrs->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    pdll_info->dwFileSize               = pdll_info->pImgNtHdrs->OptionalHeader.SizeOfImage;
    pdll_info->pImgSecHdr               = IMAGE_FIRST_SECTION(pdll_info->pImgNtHdrs);

    pdll_info->pEntryImportDataDir      = &(pdll_info->pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);
    pdll_info->pEntryExportDataDir      = &(pdll_info->pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
    pdll_info->pEntryBaseRelocDataDir   = &(pdll_info->pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
    pdll_info->pEntryTLSDataDir         = &(pdll_info->pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS]);
    pdll_info->pEntryExceptionDataDir   = &(pdll_info->pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION]);
    pdll_info->InitComplete             = true;

    return true;
}


bool
tasking::remap_dll_sections(dll_info* pdll_info) {

    if(!pdll_info->InitComplete) {
        return false;
    }

    pdll_info->pMappedData = static_cast<uint8_t*>(VirtualAlloc(
        nullptr,
        pdll_info->dwFileSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    ));

    if(!pdll_info->pMappedData) {
        return false;
    }

    for(uint16_t i = 0; i < pdll_info->pImgNtHdrs->FileHeader.NumberOfSections; i++) {
        std::memcpy(
            pdll_info->pMappedData + (pdll_info->pImgSecHdr[i].VirtualAddress),
            pdll_info->pFileBuffer + (pdll_info->pImgSecHdr[i].PointerToRawData),
            pdll_info->pImgSecHdr[i].SizeOfRawData
        );
    }

    return true;
}


bool
tasking::fix_dll_memory_permissions(dll_info *pdll_info) {

	if(!pdll_info->InitComplete || pdll_info->pMappedData == nullptr) {
		return false;
	}

	PIMAGE_SECTION_HEADER pimage_section_header = pdll_info->pImgSecHdr;
	for(size_t i = 0; i < pdll_info->pImgNtHdrs->FileHeader.NumberOfSections; i++) {

		if(!pimage_section_header[i].VirtualAddress || !pimage_section_header[i].SizeOfRawData) {
			continue;
		}

		uint32_t old_protect = 0;
		uint32_t new_protect = 0;

		if (pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE) {
			new_protect = PAGE_WRITECOPY;
		}

		if(pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_READ) {
			new_protect = PAGE_READONLY;
		}

		if ((pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE) &&
			(pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_READ)) {

			new_protect = PAGE_READWRITE;
		}

		if (pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
			new_protect = PAGE_EXECUTE;
		}

		if ((pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
			(pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE)) {

			new_protect = PAGE_EXECUTE_WRITECOPY;
		}

		if ((pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
			(pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_READ)) {

			new_protect = PAGE_EXECUTE_READ;
		}

		if ((pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
			(pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE) &&
			(pimage_section_header[i].Characteristics & IMAGE_SCN_MEM_READ)) {

			new_protect = PAGE_EXECUTE_READWRITE;
		}

		if(!VirtualProtect(
			pdll_info->pMappedData + pimage_section_header[i].VirtualAddress,
			pimage_section_header[i].SizeOfRawData,
			new_protect,
			reinterpret_cast<PDWORD>(&old_protect)
		)) {
			return false;
		}
	}

	return true;
}


bool
tasking::resolve_dll_imports(dll_info* pdll_info) {

    if(!pdll_info->InitComplete || pdll_info->pMappedData == nullptr) {
        return false;
    }

    PIMAGE_IMPORT_DESCRIPTOR pimage_import_descriptor = nullptr;

	for (size_t i = 0; i < pdll_info->pEntryImportDataDir->Size; i += sizeof(IMAGE_IMPORT_DESCRIPTOR)) {

		pimage_import_descriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>\
			(pdll_info->pMappedData + (pdll_info->pEntryImportDataDir->VirtualAddress + i));

		if (pimage_import_descriptor->FirstThunk == 0 && pimage_import_descriptor->OriginalFirstThunk == 0) {
			break;
		}

		char*		module_name				 = reinterpret_cast<char*>(pdll_info->pMappedData + (pimage_import_descriptor->Name));
		uint64_t	name_table_offset		 = pimage_import_descriptor->OriginalFirstThunk;
		uint64_t	address_table_offset	 = pimage_import_descriptor->FirstThunk;
		size_t		thunk_array_index_offset = 0; //Determines position inside of thunk array

		HMODULE module_base = LoadLibraryA(module_name);
		if (module_base == nullptr)
			return false;

		while (true) {

			PIMAGE_THUNK_DATA poriginal_first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>\
				(pdll_info->pMappedData + (name_table_offset + thunk_array_index_offset));

			PIMAGE_THUNK_DATA pfirst_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>\
				(pdll_info->pMappedData + (address_table_offset + thunk_array_index_offset));

			PIMAGE_IMPORT_BY_NAME	pimg_import_by_name = nullptr;
			uint64_t				function_addr		= 0x00;

			//
			// Null thunks indicate the end of the array.
			//

			if (poriginal_first_thunk->u1.Function == 0 && pfirst_thunk->u1.Function == 0)
				break;

			if (IMAGE_SNAP_BY_ORDINAL(poriginal_first_thunk->u1.Ordinal)) {

				//
				// If function is imported by ordinal we need to manually determine it ourselves.
				//

				PIMAGE_NT_HEADERS tmp_nt_hdrs = reinterpret_cast<PIMAGE_NT_HEADERS>\
					((uint64_t)module_base + ((PIMAGE_DOS_HEADER)module_base)->e_lfanew);

				if (tmp_nt_hdrs->Signature != IMAGE_NT_SIGNATURE)
					break;

				PIMAGE_EXPORT_DIRECTORY tmp_img_export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>\
					(((uint64_t)module_base) + (tmp_nt_hdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress));

				uint32_t* tmp_address_arr = reinterpret_cast<uint32_t*>\
					(((uint64_t)module_base) + (tmp_img_export_directory->AddressOfFunctions));

				function_addr = (uint64_t)(((uint64_t)module_base) + (tmp_address_arr[poriginal_first_thunk->u1.Ordinal]));
			}

			else {

				//
				// Function is imported by name.
				//

				pimg_import_by_name = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>\
					(pdll_info->pMappedData + (poriginal_first_thunk->u1.AddressOfData));

				function_addr = reinterpret_cast<uint64_t>\
					(GetProcAddress(module_base, pimg_import_by_name->Name));
			}

			if (!function_addr)
				return false;

			pfirst_thunk->u1.Function = function_addr;
			thunk_array_index_offset += sizeof(IMAGE_THUNK_DATA);
		}
	}

	return true;
}


bool
tasking::handle_dll_relocations(dll_info* pdll_info) {

    if (!pdll_info->InitComplete || pdll_info->pMappedData == nullptr)
        return false;

    auto pimage_base_relocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>\
        (pdll_info->pMappedData + (pdll_info->pEntryBaseRelocDataDir->VirtualAddress));

    auto delta_offset = static_cast<uint64_t>\
        (((uint64_t)(pdll_info->pMappedData)) - (pdll_info->pImgNtHdrs->OptionalHeader.ImageBase));

    PBASE_RELOCATION_ENTRY pbase_relocation_entry = nullptr;

    while(pimage_base_relocation->VirtualAddress) {

        pbase_relocation_entry = reinterpret_cast<PBASE_RELOCATION_ENTRY>(pimage_base_relocation + 1);
        while((byte*)pbase_relocation_entry != (byte*)pimage_base_relocation + pimage_base_relocation->SizeOfBlock) {

            switch(pbase_relocation_entry->Type) {
                case IMAGE_REL_BASED_DIR64:
                    *((uint64_t*)(pdll_info->pMappedData + (pimage_base_relocation->VirtualAddress + pbase_relocation_entry->Offset))) += delta_offset;
                    break;

                case IMAGE_REL_BASED_HIGHLOW:
                    *((uint32_t*)(pdll_info->pMappedData + (pimage_base_relocation->VirtualAddress + pbase_relocation_entry->Offset))) += (uint32_t)delta_offset;
                    break;

                case IMAGE_REL_BASED_HIGH:
                    *((uint16_t*)(pdll_info->pMappedData + (pimage_base_relocation->VirtualAddress + pbase_relocation_entry->Offset))) += HIWORD(delta_offset);
                    break;

                case IMAGE_REL_BASED_LOW:
                    *((uint16_t*)(pdll_info->pMappedData + (pimage_base_relocation->VirtualAddress + pbase_relocation_entry->Offset))) += LOWORD(delta_offset);
                    break;

                default:
                    break;
            }

            pbase_relocation_entry++;
        }

        pimage_base_relocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pbase_relocation_entry);
    }

    return true;
}


void ExecuteMain(fnDllMain pDllMain) {
	pDllMain(nullptr, DLL_PROCESS_ATTACH, nullptr);
}

std::string
tasking::rundll(const std::string& file_buffer) {

	dll_info dll;
	CONTEXT		ctx_cur_thread	= { 0 };
	CONTEXT		ctx_dllmain		= { 0 };

	if(!initialize_dll_info(file_buffer, &dll)) {
	    return "failed to initialize DLL info.";
	}

	if(!remap_dll_sections(&dll)) {
	    return "failed to remap DLL sections.";
	}

	if(!handle_dll_relocations(&dll)) {
	    return "failed to fix DLL relocation entries.";
	}

	if(!resolve_dll_imports(&dll)) {
		return "failed to resolve DLL imports.";
	}

	if(!fix_dll_memory_permissions(&dll)) {
		return "failed to assign correct memory protections for image sections.";
	}

	fnDllMain pmain		= reinterpret_cast<fnDllMain>(dll.pMappedData + (dll.pImgNtHdrs->OptionalHeader.AddressOfEntryPoint));
	pmain(nullptr, DLL_PROCESS_ATTACH, nullptr);



	return "success";
}
