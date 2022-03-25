#include "Common.h"
#include <olectl.h>
#pragma comment(lib, "ole32.lib")

HBITMAP LoadPictureFromFileDx(LPCTSTR pszFileName)
{
    HANDLE hFile;
    DWORD cb, cbRead;
    HGLOBAL hMem = NULL;
    HBITMAP hbm = NULL;
    LPVOID pMem = NULL;
    IStream *pifStream = NULL;
	IPicture *pifPic = NULL;

    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;

    cb = GetFileSize(hFile, NULL);
    if (cb == 0xFFFFFFFF)
        goto Quit;

    hMem = GlobalAlloc(GMEM_MOVEABLE, cb);
    pMem = GlobalLock(hMem);
    if (!pMem)
        goto Quit;

    if (!ReadFile(hFile, pMem, cb, &cbRead, NULL) || cb != cbRead)
        goto Quit;

    GlobalUnlock(hMem);
    pMem = NULL;
    hMem = NULL;

    if (CreateStreamOnHGlobal(hMem, TRUE, &pifStream) == S_OK)
    {
        OleLoadPicture(pifStream, GlobalSize(hMem), FALSE, &IID_IPicture, (LPVOID*)&pifPic);
        pifStream->lpVtbl->Release(pifStream);
    }

    if (pifPic)
    {
        OLE_HANDLE hOle;
        BITMAP bm;

        pifPic->lpVtbl->get_Handle(pifPic, &hOle);

        if (GetObject((HANDLE)hOle, sizeof(BITMAP), &bm))
        {
            HDC hDC = CreateCompatibleDC(NULL);
            if (hDC)
            {
                OLE_XSIZE_HIMETRIC cxHimetric;
                OLE_YSIZE_HIMETRIC cyHimetric;
                LONG cx, cy;

                pifPic->lpVtbl->get_Width(pifPic, &cxHimetric);
                pifPic->lpVtbl->get_Height(pifPic, &cyHimetric);
                cx = (LONG)(cxHimetric / 26.46);
                cy = (LONG)(cyHimetric / 26.46);

                pifPic->lpVtbl->Render(pifPic, hDC, 0, 0, cx, cy,
                    0, 0, cxHimetric, cyHimetric, NULL);

                DeleteDC(hDC);
            }
        }

        pifPic->lpVtbl->Release(pifPic);
    }

Quit:
    if (pMem)
        GlobalUnlock(hMem);
    if (hMem)
        GlobalFree(hMem);
    CloseHandle(hFile);
}
