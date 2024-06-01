
#include "beacon_api.hpp"
#include <stdio.h>

void BeaconDataParse(datap* parser, char* buffer, int size) {
    if (parser == nullptr) {
        return;
    }

    parser->original = buffer;
    parser->buffer = buffer;
    parser->length = size - 4;
    parser->size = size - 4;
    parser->buffer += 4;
}

int BeaconDataInt(datap* parser) {
    int fourbyteint = 0;
    if (parser->length < 4) {
        return 0;
    }
    memcpy(&fourbyteint, parser->buffer, 4);
    parser->buffer += 4;
    parser->length -= 4;
    return fourbyteint;
}

short BeaconDataShort(datap* parser) {
    short retvalue = 0;
    if (parser->length < 2) {
        return 0;
    }
    memcpy(&retvalue, parser->buffer, 2);
    parser->buffer += 2;
    parser->length -= 2;
    return retvalue;
}

int BeaconDataLength(datap* parser) {
    return parser->length;
}

char* BeaconDataExtract(datap* parser, int* size) {
    int   length  = 0;
    char* outdata = nullptr;

    if (parser->length < 4) {
        return nullptr;
    }

    memcpy(&length, parser->buffer, 4);
    parser->buffer += 4;

    outdata = parser->buffer;
    if (outdata == nullptr) {
        return nullptr;
    }

    parser->length -= 4;
    parser->length -= length;
    parser->buffer += length;
    if (size != nullptr && outdata != nullptr) {
        *size = length;
    }

    return outdata;
}

void manip_beacon_output(
        char* str,
        const bool clear,
        const bool get,
        std::string* out
    ) {

    static std::string buff;
    if(clear) {
        buff.clear();
    }
    else if(get) {
        if(out != nullptr) {
            *out = buff;
        }
    }
    else {
        buff += str;
    }
}

void clear_beacon_output() {
    manip_beacon_output(nullptr, true, false, nullptr);
}

std::string get_beacon_output() {
    std::string out;
    manip_beacon_output(nullptr, false, true, &out);
    return out;
}

void BeaconOutput(int type, char* data, int len) {
    manip_beacon_output(data, false, false, nullptr);
}

void BeaconPrintf(int type, char* fmt, ...) {
    va_list VaList  = nullptr;
    char*   buff    = nullptr;

    va_start(VaList, fmt);
    int len = vsnprintf(nullptr, 0, fmt, VaList);
    if(!len) {
        va_end(VaList);
        return;
    }

    buff = static_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1));
    if(!buff) {
        va_end(VaList);
        return;
    }

    vsnprintf(buff, len, fmt, VaList);
    manip_beacon_output(buff, false, false, nullptr);

    va_end(VaList);
    memset(buff, 0, len);
    HeapFree(GetProcessHeap(), 0, buff);
}