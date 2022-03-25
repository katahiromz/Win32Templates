#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// Recommended (gdiplus.dll)
HBITMAP LoadImageFromFileDx(LPCTSTR pszFileName);
BOOL SaveImageToFileDx(LPCTSTR pszFileName, HBITMAP hbm);

// Not recommeded (ole32.dll)
HBITMAP LoadPictureFromFileDx(LPCTSTR pszFileName);

#ifdef __cplusplus
}
#endif
