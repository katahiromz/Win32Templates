#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

LPTSTR LoadStringDx(INT nID); // no free
LONG RegMakeDx(HKEY hKey, LPCTSTR name, HKEY *phkeyResult); // RegCloseKey
VOID CenterWindowDx(HWND hwnd);
INT MsgBoxDx(HWND hwnd, LPCTSTR text, LPCTSTR title, UINT uType);
INT ErrorBoxDx(HWND hwnd, LPCTSTR text);
VOID RepositionWindowDx(HWND hwnd);
BOOL EnableProcessPrivilegeDx(LPCTSTR pszSE_);
LPTSTR MakeFilterDx(LPTSTR psz);
LPSTR GetWindowTextDxA(HWND hwnd); // free
LPWSTR GetWindowTextDxW(HWND hwnd); // free
#define GetDlgItemTextDxA(hwnd, id) GetWindowTextDxA(GetDlgItem((hwnd), (id))) // free
#define GetDlgItemTextDxW(hwnd, id) GetWindowTextDxW(GetDlgItem((hwnd), (id))) // free
INT ReadFromFileDx(LPCTSTR filename, void **ppData, size_t *psize); // free
INT WriteToFileDx(LPCTSTR filename, const void *pData, size_t size);
double getDlgItemDouble(HWND hDlg, INT nItemID, BOOL *pTranslated);
BOOL setDlgItemDouble(HWND hDlg, INT nItemID, double eValue, LPCTSTR pszFormat);

void AssertDx(const char *text, const char *file, int line);
void DebugPrintfDxA(const char *fmt, ...);
void DebugPrintfDxW(const wchar_t *fmt, ...);
void HexDumpDx(const void *ptr, size_t size, size_t top_addr OPTIONAL);
INT EnableAutoAssertDx(BOOL bEnable);

#ifdef NDEBUG
    #define TRACEA
    #define TRACEW
    #define ASSERT(x)
#else
    #define TRACEA DebugPrintfDxA
    #define TRACEW DebugPrintfDxW
    #define ASSERT(x) do { \
        if (!(x)) AssertDx(#x, __FILE__, __LINE__); \
    } while (0)
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

#if (__cplusplus >= 201103L)
    #define STATIC_ASSERT(exp) static_assert((exp), "")
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    #define STATIC_ASSERT(exp) _Static_assert((exp), "")
#else
    #define STATIC_ASSERT(exp) do { \
        typedef char STATIC_ASSERT_DUMMY_TYPE_##__LINE__[(exp) ? 1 : -1]; \
    } while (0)
#endif

void RebootDx(BOOL bForce);
void LogOffDx(BOOL bForce);
void PowerOffDx(BOOL bForce);
void ShutDownDx(BOOL bForce);

DWORD GetComCtl32VersionDx(VOID);

#define ARRAY_FOREACH(TYPE_var, array, block) do { \
    size_t ARRAY_FOREACH_##__LINE__ = 0; \
    for (; ARRAY_FOREACH_##__LINE__ < _countof(array); ++ARRAY_FOREACH_##__LINE__) { \
        TYPE_var = array[ARRAY_FOREACH_##__LINE__]; \
        block \
    } \
} while (0)

SIZE GetTextExtentDx(LPCTSTR pszText, HFONT hFont);

#ifdef __cplusplus
}
#endif
