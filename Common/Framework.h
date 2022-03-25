#pragma once

extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern LPTSTR g_pszExeName;
extern LPTSTR g_pszAppName;
extern LPTSTR g_pszRegistryKey;
extern LPTSTR g_pszProfileName;
extern LPTSTR g_pszHelpName;

void doInitFramework(void);
void doSetRegistryKey(LPCTSTR pszKey);
void doFreeFramework(void);

HKEY doGetAppRegistryKey(void);
HKEY doGetSectionKey(LPCTSTR pszSection);

#define DX_PROFILE_VALUE_MAX 512
BOOL    loadProfileInt(LPCTSTR pszSection, LPCTSTR pszEntry, INT nDefault);
LPTSTR  loadProfileSz(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszDefault);
BOOL    loadProfileBinary(LPCTSTR pszSection, LPCTSTR pszEntry, LPBYTE* ppData, LPDWORD pBytes);
BOOL saveProfileInt(LPCTSTR pszSection, LPCTSTR pszEntry, INT nValue);
BOOL saveProfileSz(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszValue);
BOOL saveProfileBinary(LPCTSTR pszSection, LPCTSTR pszEntry, LPCVOID pData, DWORD nBytes);

#define doDelRegTree RegDeleteTree
