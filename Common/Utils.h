#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

LPTSTR LoadStringDx(INT nID);
LONG RegMakeDx(HKEY hKey, LPCTSTR name, HKEY *phkeyResult);
VOID CenterWindowDx(HWND hwnd);
INT MsgBoxDx(HWND hwnd, LPCTSTR text, LPCTSTR title, UINT uType);
INT ErrorBoxDx(HWND hwnd, LPCTSTR text);
VOID RepositionWindowDx(HWND hwnd);
BOOL EnableProcessPrivilegeDx(LPCTSTR pszSE_);
LPTSTR MakeFilterDx(LPTSTR psz);
LPSTR GetWindowTextDxA(HWND hwnd);
LPWSTR GetWindowTextDxW(HWND hwnd);
#define GetDlgItemTextDxA(hwnd, id) GetWindowTextDxA(GetDlgItem((hwnd), (id)))
#define GetDlgItemTextDxW(hwnd, id) GetWindowTextDxW(GetDlgItem((hwnd), (id)))
INT ReadFromFileDx(LPCTSTR filename, void **ppData, size_t *psize);
INT WriteToFileDx(LPCTSTR filename, const void *pData, size_t size);
void AssertDx(const char *text, const char *file, int line);
void DebugPrintfDxA(const char *fmt, ...);
void DebugPrintfDxW(const wchar_t *fmt, ...);

#ifdef NDEBUG
    #define TRACEA
    #define TRACEW
    #define ASSERT(x)
    #define VERIFY(x) do { \
        if (!(x)) AssertDx(#x, __FILE__, __LINE__); \
    } while (0)
#else
    #define TRACEA DebugPrintfDxA
    #define TRACEW DebugPrintfDxW
    #define ASSERT(x) do { \
        if (!(x)) AssertDx(#x, __FILE__, __LINE__); \
    } while (0)
    #define VERIFY ASSERT
#endif

#ifdef UNICODE
    #define TRACE TRACEW
    #define GetWindowTextDx GetWindowTextDxW
    #define GetDlgItemTextDx GetDlgItemTextDxW
#else
    #define TRACE TRACEA
    #define GetWindowTextDx GetWindowTextDxA
    #define GetDlgItemTextDx GetDlgItemTextDxA
#endif

#define CP_JAPANESE 932 // Shift_JIS

// nCodePage == CP_ACP, CP_UTF8, or CP_JAPANESE.
LPWSTR WideFromAnsi(LPCSTR pszAnsi, UINT nCodePage OPTIONAL);
LPSTR AnsiFromWide(LPCWSTR pszWide, UINT nCodePage OPTIONAL);
#ifdef _WIN32
    #define AnsiFromAnsi(text, nCodePage) _strdup(text)
    #define WideFromWide(text, nCodePage) _wcsdup(text)
#else
    #define AnsiFromAnsi(text, nCodePage) strdup(text)
    #define WideFromWide(text, nCodePage) wcsdup(text)
#endif

    LPWSTR A2W(LPCSTR psz, UINT nCodePage OPTIONAL);
LPSTR W2A(LPCWSTR psz, UINT nCodePage OPTIONAL);
#define A2A(psz) psz
#define W2W(psz) psz

#ifdef UNICODE
    #define TextFromAnsi WideFromAnsi
    #define TextFromWide WideFromWide
    #define AnsiFromText AnsiFromWide
    #define WideFromText WideFromWide
    #define TextFromText WideFromWide
    #define T2A W2A
    #define T2W W2W
    #define A2T A2W
    #define W2T W2A
    #define T2T W2W
#else
    #define TextFromAnsi AnsiFromAnsi
    #define TextFromWide AnsiFromWide
    #define AnsiFromText AnsiFromAnsi
    #define WideFromText WideFromAnsi
    #define TextFromText AnsiFromAnsi
    #define T2A A2A
    #define T2W A2W
    #define A2T A2A
    #define W2T W2A
    #define T2T A2A
#endif

void RebootDx(BOOL bForce);
void LogOffDx(BOOL bForce);
void PowerOffDx(BOOL bForce);
void ShutDownDx(BOOL bForce);

DWORD GetComCtl32VersionDx(VOID);

#ifdef __cplusplus
}
#endif
