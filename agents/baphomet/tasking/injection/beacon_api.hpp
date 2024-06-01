//
// vendor.
//

#ifndef BEACON_API_HPP
#define BEACON_API_HPP
#include <Windows.h>
#include <string>

typedef struct {
    char* original; /* the original buffer [so we can free it] */
    char* buffer;   /* current pointer into our buffer */
    int    length;  /* remaining length of data */
    int    size;    /* total size of this buffer */
} datap;

void    BeaconDataParse(datap* parser, char* buffer, int size);
int     BeaconDataInt(datap* parser);
short   BeaconDataShort(datap* parser);
int     BeaconDataLength(datap* parser);
char*   BeaconDataExtract(datap* parser, int* size);

#define CALLBACK_OUTPUT      0x0
#define CALLBACK_OUTPUT_OEM  0x1e
#define CALLBACK_OUTPUT_UTF8 0x20
#define CALLBACK_ERROR       0x0d

void BeaconOutput(int type, char* data, int len);
void BeaconPrintf(int type, char* fmt, ...);

std::string get_beacon_output();
void clear_beacon_output();
void manip_beacon_output(char* str, bool clear, bool get, std::string* out);

#endif //BEACON_API_HPP
