#include "Common.h"
#ifdef __cplusplus
    #include <gdiplus.h>
#else
    #include "fakegdiplus.h"
#endif
#pragma comment(lib, "gdiplus.lib")

HBITMAP LoadImageFromFileDx(LPCTSTR pszFileName)
{
    HBITMAP hbm = NULL;
    LPWSTR pszWideFileName = WideFromText(CP_ACP, pszFileName);
#ifdef __cplusplus
    using namespace Gdiplus;
    {
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        Bitmap* pBitmap = Gdiplus::Bitmap::FromFile(pszWideFileName);
        if (pBitmap)
        {
            pBitmap->GetHBITMAP(Color::MakeARGB(255, 255, 255, 255), &hbm);
            delete pBitmap;
        }

        GdiplusShutdown(gdiplusToken);
    }
#else
    {
        GdiplusStartupInput gdiplusStartupInput = { FALSE };
        ULONG_PTR gdiplusToken;
        GpBitmap *pBitmap = NULL;

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        GdipCreateBitmapFromFile(pszWideFileName, &pBitmap);
        if (pBitmap)
        {
            GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, MakeARGB(255, 255, 255, 255));
            GdipDisposeImage(pBitmap);
        }

        GdiplusShutdown(gdiplusToken);
    }
#endif
    free(pszWideFileName);
    return hbm;
}

BOOL SaveImageToFileDx(LPCTSTR pszFileName, HBITMAP hbm)
{
    BOOL ret = FALSE;
    LPWSTR pszWideFileName = WideFromText(CP_ACP, pszFileName);
    LPWSTR pchDotExt = PathFindExtensionW(pszWideFileName);
    CLSID clsid;
    if (lstrcmpiW(pchDotExt, L".bmp") == 0)
    {
        CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &clsid);
    }
    else if (lstrcmpiW(pchDotExt, L".png") == 0)
    {
        CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    }
    else if (lstrcmpiW(pchDotExt, L".jpg") == 0 || lstrcmpiW(pchDotExt, L".jpeg") == 0)
    {
        CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &clsid);
    }
    else if (lstrcmpiW(pchDotExt, L".gif") == 0)
    {
        CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &clsid);
    }
    else if (lstrcmpiW(pchDotExt, L".tif") == 0 || lstrcmpiW(pchDotExt, L".tiff") == 0)
    {
        CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &clsid);
    }
    else
    {
        free(pszWideFileName);
        return FALSE;
    }
#ifdef __cplusplus
    {
        using namespace Gdiplus;
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GpStatus status;

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        Bitmap *pBitmap = Bitmap::FromHBITMAP(hbm, NULL);
        if (pBitmap)
        {
            status = pBitmap->Save(pszWideFileName, &clsid, NULL);
            delete pBitmap;

            ret = (status == Ok);
        }

        GdiplusShutdown(gdiplusToken);
    }
#else
    {
        GdiplusStartupInput gdiplusStartupInput = { FALSE };
        ULONG_PTR gdiplusToken;
        GpStatus status;
        GpBitmap *pBitmap = NULL;

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        GdipCreateBitmapFromHBITMAP(hbm, NULL, &pBitmap);
        if (pBitmap)
        {
            status = GdipSaveImageToFile(pBitmap, pszWideFileName, &clsid, NULL);
            GdipDisposeImage(pBitmap);

            ret = (status == 0);
        }

        GdiplusShutdown(gdiplusToken);
    }
#endif
    free(pszWideFileName);
    return ret;
}
