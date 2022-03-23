#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

BOOL CreateShortcutDx(LPCTSTR pszLnkFileName,
                      LPCTSTR pszTargetPathName,
                      LPCTSTR pszDescription OPTIONAL);
BOOL GetPathOfShortcutDx(HWND hWnd, LPCTSTR pszLnkFile, LPTSTR pszPath);

BOOL CreateInternetShortcutDx(LPCTSTR pszUrlFileName, LPCTSTR pszURL);
BOOL GetURLOfInternetShortcutDx(LPCTSTR pszUrlFileName, LPTSTR *ppszURL); // CoTaskMemFree

#ifdef __cplusplus
}
#endif
