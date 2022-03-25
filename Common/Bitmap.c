#include "Common.h"

typedef struct tagBITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} BITMAPINFOEX, FAR * LPBITMAPINFOEX;

HBITMAP LoadBitmapFromFileDx(LPCTSTR pszFileName)
{
    HANDLE hFile;
    BITMAPFILEHEADER bf;
    BITMAPINFOEX bi;
    DWORD cb, cbImage;
    LPVOID pBits;
    HBITMAP hbm;

    hbm = (HBITMAP)LoadImage(NULL, pszFileName, IMAGE_BITMAP, 0, 0,
                             LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hbm != NULL)
        return hbm;

    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;

    if (!ReadFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL))
    {
        CloseHandle(NULL);
        return NULL;
    }

    pBits = NULL;
    if (bf.bfType == 0x4D42 && bf.bfReserved1 == 0 && bf.bfReserved2 == 0 &&
        bf.bfSize > bf.bfOffBits && bf.bfOffBits > sizeof(BITMAPFILEHEADER) &&
        bf.bfOffBits <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOEX))
    {
        cbImage = bf.bfSize - bf.bfOffBits;
        pBits = malloc(cbImage);
        if (pBits)
        {
            if (!ReadFile(hFile, &bi, bf.bfOffBits - sizeof(BITMAPFILEHEADER), &cb, NULL) ||
                !ReadFile(hFile, pBits, cbImage, &cb, NULL))
            {
                free(pBits);
                pBits = NULL;
            }
        }
    }

    CloseHandle(hFile);

    if (pBits == NULL)
        return NULL;

    hbm = CreateDIBSection(NULL, (LPBITMAPINFO)&bi, DIB_RGB_COLORS, NULL, NULL, 0);
    if (hbm)
    {
        if (!SetDIBits(NULL, hbm, 0, abs(bi.bmiHeader.biHeight), pBits,
                       (LPBITMAPINFO)&bi, DIB_RGB_COLORS))
        {
            DeleteObject(hbm);
            hbm = NULL;
        }
    }

    free(pBits);
    return hbm;
}

BOOL SaveBitmapToFileDx(LPCTSTR pszFileName, HBITMAP hbm)
{
    BOOL f;
    BITMAPFILEHEADER bf;
    BITMAPINFOEX bi;
    BITMAPINFOHEADER *pbi;
    DWORD cb;
    DWORD cColors, cbColors;
    HDC hDC;
    HANDLE hFile;
    LPVOID pBits;
    BITMAP bm;

    if (!GetObject(hbm, sizeof(BITMAP), &bm))
        return FALSE;

    pbi = &bi.bmiHeader;
    ZeroMemory(pbi, sizeof(BITMAPINFOHEADER));
    pbi->biSize             = sizeof(BITMAPINFOHEADER);
    pbi->biWidth            = bm.bmWidth;
    pbi->biHeight           = bm.bmHeight;
    pbi->biPlanes           = 1;
    pbi->biBitCount         = bm.bmBitsPixel;
    pbi->biCompression      = BI_RGB;
    pbi->biSizeImage        = bm.bmWidthBytes * bm.bmHeight;

    if (bm.bmBitsPixel < 16)
        cColors = 1 << bm.bmBitsPixel;
    else
        cColors = 0;
    cbColors = cColors * sizeof(RGBQUAD);

    bf.bfType = 0x4D42;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    cb = sizeof(BITMAPFILEHEADER) + pbi->biSize + cbColors;
    bf.bfOffBits = cb;
    bf.bfSize = cb + pbi->biSizeImage;

    pBits = malloc(pbi->biSizeImage);
    if (pBits == NULL)
        return FALSE;

    f = FALSE;
    hDC = CreateCompatibleDC(NULL);
    if (hDC)
    {
        if (GetDIBits(hDC, hbm, 0, bm.bmHeight, pBits, (LPBITMAPINFO)&bi, DIB_RGB_COLORS))
        {
            hFile = CreateFile(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                               FILE_FLAG_WRITE_THROUGH, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                f = WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL) &&
                    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &cb, NULL) &&
                    WriteFile(hFile, bi.bmiColors, cbColors, &cb, NULL) &&
                    WriteFile(hFile, pBits, pbi->biSizeImage, &cb, NULL);
                CloseHandle(hFile);

                if (!f)
                    DeleteFile(pszFileName);
            }
        }
        DeleteDC(hDC);
    }

    free(pBits);
    return f;
}

HBITMAP Create24BppBitmapDx(INT width, INT height)
{
    HDC hDC;
    BITMAPINFO bmi;
    HBITMAP hbm;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    hDC = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
    DeleteDC(hDC);
    return hbm;
}

HBITMAP Create32BppBitmapDx(INT width, INT height)
{
    HDC hDC;
    BITMAPINFO bmi;
    HBITMAP hbm;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    hDC = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
    DeleteDC(hDC);
    return hbm;
}

#ifdef UNITTEST
#include "Utils.c"
int main(void)
{
    HBITMAP hbm = LoadBitmapFromFileDx("a.bmp");
    printf("hbm: %p\n", hbm);
    SaveBitmapToFileDx("c.bmp", hbm);
    DeleteObject(hbm);
    return 0;
}
#endif
