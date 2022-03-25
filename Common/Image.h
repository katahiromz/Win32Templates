#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

HBITMAP LoadImageFromFileDx(LPCTSTR pszFileName);
BOOL SaveImageToFileDx(LPCTSTR pszFileName, HBITMAP hbm);

#ifdef __cplusplus
}
#endif
