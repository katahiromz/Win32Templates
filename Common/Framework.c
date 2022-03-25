#include "Common.h"

LPTSTR g_pszExeName = NULL;
LPTSTR g_pszAppName = NULL;
LPTSTR g_pszRegistryKey = NULL;
LPTSTR g_pszProfileName = NULL;
LPTSTR g_pszHelpName = NULL;

void doInitFramework(void)
{
    // get g_pszExeName
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, _countof(szPath));
    ASSERT(g_pszExeName == NULL); // double call check
    g_pszExeName = _tcsdup(szPath);

    // get g_pszProfileName
    PathRemoveExtension(szPath);
    PathAddExtension(szPath, TEXT(".ini"));
    g_pszProfileName = _tcsdup(szPath);

    // get g_pszAppName
    PathRemoveExtension(szPath);
    g_pszAppName = _tcsdup(PathFindFileName(szPath));

    // get g_pszHelpName
#ifdef DX_HELPFILE
    g_pszHelpName = _tcsdup(DX_HELPFILE);
#else
    PathRemoveExtension(szPath);
    PathAddExtension(szPath, DX_HELPFILE_DOTEXT);
    g_pszHelpName = _tcsdup(szPath);
#endif
}

void doFreeFramework(void)
{
    free(g_pszExeName);
    g_pszExeName = NULL;

    free(g_pszAppName);
    g_pszAppName = NULL;

    free(g_pszRegistryKey);
    g_pszRegistryKey = NULL;

    free(g_pszProfileName);
    g_pszProfileName = NULL;

    free(g_pszHelpName);
    g_pszHelpName = NULL;
}

void doSetRegistryKey(LPCTSTR pszKey)
{
    ASSERT(pszKey);

    free(g_pszRegistryKey);
    g_pszRegistryKey = _tcsdup(pszKey);

    free(g_pszProfileName);
    g_pszProfileName = _tcsdup(g_pszAppName);
}

LONG doMakeRegKey(HKEY hKey, LPCTSTR pszName, HKEY *phkeyGot)
{
    return RegCreateKeyEx(hKey, pszName, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, phkeyGot, NULL);
}

HKEY doGetAppRegistryKey(void)
{
    HKEY hSoftwareKey, hCompanyKey, hAppKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software"), 0, KEY_READ | KEY_WRITE, &hSoftwareKey);
    if (!hSoftwareKey)
        return NULL;

    doMakeRegKey(hSoftwareKey, g_pszRegistryKey, &hCompanyKey);
    doMakeRegKey(hCompanyKey, g_pszProfileName, &hAppKey);
    RegCloseKey(hSoftwareKey);
    RegCloseKey(hCompanyKey);
    return hAppKey;
}

HKEY doGetSectionKey(LPCTSTR pszSection)
{
    HKEY hAppKey, hSectionKey = NULL;
    ASSERT(pszSection);

    hAppKey = doGetAppRegistryKey();
    if (!hAppKey)
        return NULL;

    doMakeRegKey(hAppKey, pszSection, &hSectionKey);
    RegCloseKey(hAppKey);
    return hSectionKey;
}

BOOL loadProfileInt(LPCTSTR pszSection, LPCTSTR pszEntry, INT nDefault)
{
    ASSERT(pszSection);
    ASSERT(pszEntry);

    if (g_pszRegistryKey)
    {
        LONG error;
        DWORD dwValue, dwType, cbValue = sizeof(dwValue);
        HKEY hSectionKey = doGetSectionKey(pszSection);
        if (!hSectionKey)
            return nDefault;

        error = RegQueryValueEx(hSectionKey, pszEntry, NULL, &dwType, (LPBYTE)&dwValue, &cbValue);
        RegCloseKey(hSectionKey);
        if (error)
            return nDefault;
        else
            ASSERT(dwType == REG_DWORD);

        return (INT)dwValue;
    }

    ASSERT(g_pszProfileName);
    return GetPrivateProfileInt(pszSection, pszEntry, nDefault, g_pszProfileName);
}

LPTSTR loadProfileSz(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszDefault)
{
    static TCHAR s_szValue[DX_PROFILE_VALUE_MAX];

    ASSERT(pszSection);
    ASSERT(pszEntry);

    if (g_pszRegistryKey)
    {
        LONG error;
        DWORD cbValue, dwType;
        HKEY hSectionKey = doGetSectionKey(pszSection);
        if (!hSectionKey)
        {
            StringCchCopy(s_szValue, _countof(s_szValue), pszDefault);
            return s_szValue;
        }

        cbValue = sizeof(s_szValue);
        error = RegQueryValueEx(hSectionKey, pszEntry, NULL, &dwType, (LPBYTE)s_szValue, &cbValue);
        RegCloseKey(hSectionKey);

        if (error)
            StringCchCopy(s_szValue, _countof(s_szValue), pszDefault);
        else
            ASSERT(dwType == REG_SZ);

        return s_szValue;
    }

    ASSERT(g_pszProfileName);
    if (!pszDefault)
        pszDefault = TEXT("");
    GetPrivateProfileString(pszSection, pszEntry, pszDefault, s_szValue, _countof(s_szValue), g_pszProfileName);
    return s_szValue;
}

BOOL loadProfileBinary(LPCTSTR pszSection, LPCTSTR pszEntry, LPBYTE* ppData, LPDWORD pBytes)
{
    ASSERT(pszSection);
    ASSERT(pszEntry);
    ASSERT(ppData);
    ASSERT(pBytes);

    *ppData = NULL;
    *pBytes = 0;

    if (g_pszRegistryKey)
    {
        LONG error;
        DWORD dwType, cbValue;
        HKEY hSectionKey = doGetSectionKey(pszSection);
        if (!hSectionKey)
            return FALSE;

        error = RegQueryValueEx(hSectionKey, pszEntry, NULL, &dwType, NULL, &cbValue);
        *pBytes = cbValue;
        if (!error)
        {
            ASSERT(dwType == REG_BINARY);
            *ppData = (LPBYTE)malloc(cbValue);
            error = RegQueryValueEx(hSectionKey, pszEntry, NULL, &dwType, *ppData, &cbValue);
        }

        RegCloseKey(hSectionKey);

        if (error)
        {
            free(*ppData);
            *ppData = NULL;
            *pBytes = 0;
            return FALSE;
        }
        else
        {
            ASSERT(dwType == REG_BINARY);
        }

        return TRUE;
    }
    else
    {
        LPBYTE pb;
        DWORD ib, cch, cb;
        LPTSTR psz;
        TCHAR sz[3];

        psz = loadProfileSz(pszSection, pszEntry, NULL);
        if (!psz)
            return FALSE;

        cch = lstrlen(psz);
        ASSERT(cch % 2 == 0);
        *pBytes = cb = cch / 2;

        pb = malloc(cb);
        if (!pb)
            return FALSE;

        for (ib = 0; ib < cb; ++ib)
        {
            sz[0] = psz[2 * ib + 0];
            sz[1] = psz[2 * ib + 1];
            sz[2] = 0;
            pb[ib] = (BYTE)_tcstoul(sz, NULL, 16);
        }

        *ppData = pb;
        return TRUE;
    }
}

BOOL saveProfileInt(LPCTSTR pszSection, LPCTSTR pszEntry, INT nValue)
{
    ASSERT(pszSection);
    ASSERT(pszEntry);

    if (g_pszRegistryKey)
    {
        LONG error;
        DWORD dwValue = nValue;
        HKEY hSectionKey = doGetSectionKey(pszSection);
        if (!hSectionKey)
            return FALSE;

        error = RegSetValueEx(hSectionKey, pszEntry, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        RegCloseKey(hSectionKey);
        return !error;
    }
    else
    {
        TCHAR sz[32];
        StringCchPrintf(sz, _countof(sz), TEXT("%d"), nValue);
        return WritePrivateProfileString(pszSection, pszEntry, sz, g_pszProfileName);
    }
}

BOOL saveProfileSz(LPCTSTR pszSection, LPCTSTR pszEntry, LPCTSTR pszValue)
{
    ASSERT(pszSection);

    if (g_pszRegistryKey)
    {
        LONG error;
        if (!pszSection)
        {
            HKEY hAppKey = doGetAppRegistryKey();
            if (!hAppKey)
                return FALSE;

            error = RegDeleteKey(hAppKey, pszSection);

            RegCloseKey(hAppKey);
        }
        else
        {
            HKEY hSectionKey = doGetSectionKey(pszSection);
            if (!hSectionKey)
                return FALSE;

            if (!pszValue)
            {
                error = RegDeleteValue(hSectionKey, pszEntry);
            }
            else
            {
                DWORD cbValue = (lstrlen(pszValue) + 1) * sizeof(TCHAR);
                error = RegSetValueEx(hSectionKey, pszEntry, 0, REG_SZ, (LPBYTE)pszValue, cbValue);
            }

            RegCloseKey(hSectionKey);
        }
        return !error;
    }
    else
    {
        ASSERT(g_pszProfileName);
        return WritePrivateProfileString(pszSection, pszEntry, pszValue, g_pszProfileName);
    }
}

BOOL saveProfileBinary(LPCTSTR pszSection, LPCTSTR pszEntry, LPCVOID pData, DWORD nBytes)
{
    ASSERT(pszSection);

    if (g_pszRegistryKey)
    {
        LONG error;
        HKEY hSectionKey = doGetSectionKey(pszSection);
        if (!hSectionKey)
            return FALSE;

        error = RegSetValueEx(hSectionKey, pszEntry, 0, REG_BINARY, (const BYTE *)pData, nBytes);
        RegCloseKey(hSectionKey);
        return !error;
    }
    else
    {
        static const TCHAR s_hex[] = TEXT("0123456789ABCDEF");
        LPTSTR psz = malloc((nBytes * 2 + 1) * sizeof(TCHAR));
        if (!psz)
            return FALSE;

        {
            const BYTE *pb = pData;
            DWORD ib;
            for (ib = 0; ib < nBytes; ++ib)
            {
                psz[2 * ib + 0] = s_hex[pb[ib] & 0xF];
                psz[2 * ib + 1] = s_hex[pb[ib] >> 4];
            }
            psz[2 * nBytes] = 0;
        }

        ASSERT(g_pszProfileName);

        {
            BOOL ret = saveProfileSz(pszSection, pszEntry, psz);
            free(psz);
            return ret;
        }
    }
}

PRECENT loadRecentFileList(INT nMaxRecents, LPCTSTR pszSectionName OPTIONAL)
{
    INT i;
    LPTSTR psz;
    TCHAR szName[64];
    PRECENT pRecent;

    if (pszSectionName == NULL)
        pszSectionName = TEXT("Recent File List");

    pRecent = Recent_New(nMaxRecents);
    if (!pRecent)
        return NULL;

    for (i = 0; i < nMaxRecents; ++i)
    {
        StringCchPrintf(szName, _countof(szName), TEXT("File%u"), i + 1);
        psz = loadProfileSz(pszSectionName, szName, TEXT(""));
        if (!psz || !psz[0])
            break;

        Recent_Add(pRecent, psz);
    }

    return pRecent;
}

BOOL saveRecentFileList(PRECENT pRecent, LPCTSTR pszSectionName OPTIONAL)
{
    INT i, nRecentCount = Recent_GetCount(pRecent);
    TCHAR szName[64];

    if (pszSectionName == NULL)
        pszSectionName = TEXT("Recent File List");

    for (i = 0; i < nRecentCount; ++i)
    {
        StringCchPrintf(szName, _countof(szName), TEXT("File%u"), i + 1);
        saveProfileSz(TEXT("Recent File List"), szName, Recent_GetAt(pRecent, i));
    }

    StringCchPrintf(szName, _countof(szName), TEXT("File%u"), i + 1);
    saveProfileSz(TEXT("Recent File List"), szName, NULL);
    return TRUE;
}
