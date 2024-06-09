//
// Beacon function definitions are Courtesy of: https://github.com/Cobalt-Strike/bof_template/tree/main
//

#ifndef BEACON_API_HPP
#define BEACON_API_HPP
#include <Windows.h>
#include <string>

#define CALLBACK_OUTPUT      0x0
#define CALLBACK_OUTPUT_OEM  0x1e
#define CALLBACK_OUTPUT_UTF8 0x20
#define CALLBACK_ERROR       0x0d


/* Structs */
typedef struct {
    char* original; /* the original buffer [so we can free it] */
    char* buffer;   /* current pointer into our buffer */
    int   length;  /* remaining length of data */
    int   size;    /* total size of this buffer */
} datap;

typedef struct {
    char* original; /* the original buffer [so we can free it] */
    char* buffer;   /* current pointer into our buffer */
    int   length;   /* remaining length of data */
    int   size;     /* total size of this buffer */
} formatp;


/* Beacon Data */
void    BeaconDataParse(datap* parser, char* buffer, int size);
int     BeaconDataInt(datap* parser);
short   BeaconDataShort(datap* parser);
int     BeaconDataLength(datap* parser);
char*   BeaconDataExtract(datap* parser, int* size);

/* Beacon Format */
void    BeaconFormatAlloc(formatp* format, int maxsz);
void    BeaconFormatReset(formatp* format);
void    BeaconFormatAppend(formatp* format, char* text, int len);
void    BeaconFormatPrintf(formatp* format, char* fmt, ...);
char*   BeaconFormatToString(formatp* format, int* size);
void    BeaconFormatFree(formatp* format);
void    BeaconFormatInt(formatp* format, int value);


/* Output */
void BeaconOutput(int type, char* data, int len);
void BeaconPrintf(int type, char* fmt, ...);

/* Misc */
BOOL BeaconIsAdmin();
BOOL BeaconUseToken(HANDLE token);
void BeaconRevertToken();
BOOL toWideChar(char* src, wchar_t* dst, int max);

/* Fork & run / process injection */
void   BeaconGetSpawnTo(BOOL x86, char* buffer, int length);
BOOL   BeaconSpawnTemporaryProcess(BOOL x86, BOOL ignoreToken, STARTUPINFO* si, PROCESS_INFORMATION* pInfo);
void   BeaconInjectTemporaryProcess(PROCESS_INFORMATION* pInfo, char* payload, int p_len, int p_offset, char* arg, int a_len);
void   BeaconInjectProcess(HANDLE hProc, int pid, char* payload, int p_len, int p_offset, char* arg, int a_len);
void   BeaconCleanupProcess(PROCESS_INFORMATION* pInfo);

/* Internal */
void manip_beacon_output(char* str, bool clear, bool get, std::string* out);
void manip_token(_In_ bool clear, _In_ HANDLE token, _Out_ HANDLE* out);
std::string get_beacon_output();
void clear_beacon_output();
HANDLE get_curr_token();
void clear_curr_token();
void set_curr_token(HANDLE token);
uint32_t swap_endianess(uint32_t indata);
size_t char_to_wide_impl(wchar_t* dest, char* src, size_t max_allowed);

#endif //BEACON_API_HPP
