#pragma once

typedef struct GdiplusStartupInput {
    UINT32 GdiplusVersion;
    void *DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef struct GdiplusStartupOutput {
    LPVOID dummy1, dummy2;
} GdiplusStartupOutput;

typedef INT GpStatus;
typedef void *GpBitmap;
typedef void *GpImage;
typedef DWORD ARGB;
#define MakeARGB(a, r, g, b) MAKELONG(MAKEWORD(b, g), MAKEWORD(r, a))

DECLSPEC_IMPORT GpStatus WINAPI GdiplusStartup(ULONG_PTR*, const GdiplusStartupInput*, GdiplusStartupOutput*);
DECLSPEC_IMPORT VOID WINAPI GdiplusShutdown(ULONG_PTR);
DECLSPEC_IMPORT GpStatus WINAPI GdipCreateBitmapFromFile(const WCHAR *, GpBitmap**);
DECLSPEC_IMPORT GpStatus WINAPI GdipCreateHBITMAPFromBitmap(GpBitmap*, HBITMAP*, ARGB);
DECLSPEC_IMPORT GpStatus WINAPI GdipDisposeImage(GpImage*);
DECLSPEC_IMPORT GpStatus WINAPI GdipCreateBitmapFromHBITMAP(HBITMAP, HPALETTE, GpBitmap**);
DECLSPEC_IMPORT GpStatus WINAPI GdipSaveImageToFile(GpImage*, const WCHAR*, const CLSID*, const void*);
