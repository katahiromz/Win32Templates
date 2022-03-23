#include "Common.h"
#include <shlobj.h>
#include <intshcut.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "shlwapi.lib")

BOOL CreateShortcutDx(LPCTSTR pszLnkFileName,
                      LPCTSTR pszTargetPathName,
                      LPCTSTR pszDescription OPTIONAL)
{
    HRESULT hr;
    IPersistFile* pPF;
    IShellLink* pSL;
#ifndef UNICODE
    WCHAR wsz[MAX_PATH];
#endif
    assert(lstrcmpi(PathFindExtension(pszLnkFileName), TEXT(".LNK")) == 0);
    assert(!PathIsRelative(pszTargetPathName));

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink,
                              (LPVOID*)&pSL);
        if (SUCCEEDED(hr))
        {
            pSL->lpVtbl->SetPath(pSL, pszTargetPathName);
            if (pszDescription)
                pSL->lpVtbl->SetDescription(pSL, pszDescription);

            hr = pSL->lpVtbl->QueryInterface(pSL, &IID_IPersistFile, (LPVOID*)&pPF);
            if (SUCCEEDED(hr))
            {
#ifdef UNICODE
                hr = pPF->lpVtbl->Save(pPF, pszLnkFileName, TRUE);
#else
                MultiByteToWideChar(CP_ACP, 0, pszLnkFileName, -1, wsz, _countof(wsz));
                hr = pPF->lpVtbl->Save(pPF, wsz, TRUE);
#endif
                pPF->lpVtbl->Release(pPF);
            }
            pSL->lpVtbl->Release(pSL);
        }
        CoUninitialize();
    }

    return SUCCEEDED(hr);
}

BOOL GetPathOfShortcutDx(HWND hWnd, LPCTSTR pszLnkFile, LPTSTR pszPath)
{
    BOOL ret = FALSE;
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
#ifndef UNICODE
    WCHAR wsz[MAX_PATH];
#endif
    IShellLink *pSL;
    IPersistFile *pPF;

    szPath[0] = 0;
    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(&CLSID_ShellLink, NULL, 
                              CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID *)&pSL);
        if (SUCCEEDED(hr))
        {
            hr = pSL->lpVtbl->QueryInterface(pSL, &IID_IPersistFile, (VOID **)&pPF);
            if (SUCCEEDED(hr))
            {
#ifndef UNICODE
                MultiByteToWideChar(CP_ACP, 0, pszLnkFile, -1, wsz, _countof(wsz));
                hr = pPF->lpVtbl->Load(pPF, wsz, STGM_READ);
#else
                hr = pPF->lpVtbl->Load(pPF, pszLnkFile, STGM_READ);
#endif
                if (SUCCEEDED(hr))
                {
                    hr = pSL->lpVtbl->GetPath(pSL, szPath, _countof(szPath), NULL, 0);
                    if (SUCCEEDED(hr))
                    {
                        if (szPath[0])
                        {
                            StringCchCopy(pszPath, MAX_PATH, szPath);
                            ret = TRUE;
                        }
                    }
                }
                pPF->lpVtbl->Release(pPF);
            }
            pSL->lpVtbl->Release(pSL);
        }
        CoUninitialize();
    }

    return ret;
}

BOOL CreateInternetShortcutDx(LPCTSTR pszUrlFileName, LPCTSTR pszURL)
{
    IPersistFile* pPF;
    IUniformResourceLocator* pURL;
    HRESULT hr;
#ifndef UNICODE
    WCHAR   wsz[MAX_PATH];
#endif
    assert(!PathIsRelative(pszUrlFileName));
    assert(lstrcmpi(PathFindExtension(pszUrlFileName), TEXT(".url")) == 0);

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(&CLSID_InternetShortcut, NULL,
                              CLSCTX_INPROC_SERVER, &IID_IUniformResourceLocator,
                              (LPVOID*)&pURL);
        if (SUCCEEDED(hr))
        {
            pURL->lpVtbl->SetURL(pURL, pszURL, 0);

            hr = pURL->lpVtbl->QueryInterface(pURL, &IID_IPersistFile, (LPVOID*)&pPF);
            if (SUCCEEDED(hr))
            {
#ifdef UNICODE
                hr = pPF->lpVtbl->Save(pPF, pszUrlFileName, TRUE);
#else
                MultiByteToWideChar(CP_ACP, 0, pszUrlFileName, -1, wsz, _countof(wsz));
                hr = pPF->lpVtbl->Save(pPF, wsz, TRUE);
#endif
                pPF->lpVtbl->Release(pPF);
            }
            pURL->lpVtbl->Release(pURL);
        }
        CoUninitialize();
    }

    return SUCCEEDED(hr);
}

BOOL GetURLOfInternetShortcutDx(LPCTSTR pszUrlFileName, LPTSTR *ppszURL)
{
    IPersistFile* pPF;
    IUniformResourceLocator* pURL;
    HRESULT hr;
    BOOL ret = FALSE;
#ifndef UNICODE
    WCHAR   wsz[MAX_PATH];
#endif
    assert(!PathIsRelative(pszUrlFileName));
    assert(lstrcmpi(PathFindExtension(pszUrlFileName), TEXT(".url")) == 0);
    *ppszURL = NULL;

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(&CLSID_InternetShortcut, NULL,
                              CLSCTX_INPROC_SERVER, &IID_IUniformResourceLocator,
                              (LPVOID*)&pURL);
        if (SUCCEEDED(hr))
        {
            hr = pURL->lpVtbl->QueryInterface(pURL, &IID_IPersistFile, (LPVOID*)&pPF);
            if (SUCCEEDED(hr))
            {
#ifdef UNICODE
                hr = pPF->lpVtbl->Load(pPF, pszUrlFileName, STGM_READ | STGM_SHARE_DENY_NONE);
#else
                MultiByteToWideChar(CP_ACP, 0, pszUrlFileName, -1, wsz, _countof(wsz));
                hr = pPF->lpVtbl->Load(pPF, wsz, STGM_READ | STGM_SHARE_DENY_NONE);
#endif
                if (SUCCEEDED(hr))
                {
                    hr = pURL->lpVtbl->GetURL(pURL, ppszURL);
                    ret = (SUCCEEDED(hr) && hr != S_FALSE);
                }
                pPF->lpVtbl->Release(pPF);
            }
            pURL->lpVtbl->Release(pURL);
        }
        CoUninitialize();
    }

    return ret;
}

#ifdef UNITTEST
int main(void)
{
    char path[MAX_PATH];
    char *url;
    GetFullPathNameA("Shortcut.c", MAX_PATH, path, 0);
    CreateShortcutDx("Shortcut to Shortcut.c.lnk", path, NULL);
    path[0] = 0;
    GetPathOfShortcutDx(NULL, "Shortcut to Shortcut.c.lnk", path);
    printf("%s\n", path);
    GetFullPathNameA("google.com.url", MAX_PATH, path, 0);
    CreateInternetShortcutDx(path, "https://google.com");
    GetURLOfInternetShortcutDx(path, &url);
    printf("%s\n", url);
    CoTaskMemFree(url);
}
#endif
