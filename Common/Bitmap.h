#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define GetBitmapInfoDx(hbm, pbm) GetObject((hbm), sizeof(BITMAP), (pbm))

HBITMAP LoadBitmapFromFileDx(LPCTSTR pszFileName);
BOOL SaveBitmapToFileDx(LPCTSTR pszFileName, HBITMAP hbm);

HBITMAP Create24BppBitmapDx(INT width, INT height);
HBITMAP Create32BppBitmapDx(INT width, INT height);

HBITMAP LoadImageFromFileDx(LPCTSTR pszFileName);
BOOL SaveImageToFileDx(LPCTSTR pszFileName, HBITMAP hbm);

#ifdef __cplusplus
}
#endif
