#pragma once

#include "Recent.h"

#define DX_COMPANY_NAME_IN_ENGLISH TEXT("Katayama Hirofumi MZ")

//#define DX_HELPFILE TEXT("README.txt")
//#define DX_HELPFILE TEXT("README.htm")
#define DX_HELPFILE_DOTEXT TEXT(".TXT")

#ifdef __cplusplus
extern "C"
{
#endif

extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern LPTSTR g_pszExeName;
extern LPTSTR g_pszAppName;
extern LPTSTR g_pszRegistryKey;
extern LPTSTR g_pszProfileName;
extern LPTSTR g_pszHelpName;

void doInitFramework(void); // doFreeFramework
void doSetRegistryKey(LPCTSTR pszKey);
void doFreeFramework(void);

HKEY doGetAppRegistryKey(void);
HKEY doGetSectionKey(LPCTSTR pszSection);

#define DX_PROFILE_VALUE_MAX 512

BOOL    loadProfileInt(LPCTSTR pszSection, LPCTSTR pszEntry, INT nDefault);
LPTSTR  loadProfileSz(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszDefault);
BOOL    loadProfileBinary(LPCTSTR pszSection, LPCTSTR pszEntry, LPBYTE* ppData, LPDWORD pBytes);
PRECENT loadRecentFileList(INT nMaxRecents, LPCTSTR pszSectionName OPTIONAL);
BOOL saveProfileInt(LPCTSTR pszSection, LPCTSTR pszEntry, INT nValue);
BOOL saveProfileSz(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszValue);
BOOL saveProfileBinary(LPCTSTR pszSection, LPCTSTR pszEntry, LPCVOID pData, DWORD nBytes);
BOOL saveRecentFileList(PRECENT pRecent, LPCTSTR pszSectionName OPTIONAL);

#define doDelRegTree RegDeleteTree

#ifdef __cplusplus
} // extern "C"
#endif
