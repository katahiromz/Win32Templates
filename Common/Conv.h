#pragma once

#ifdef __cplusplus
extern "C"
{
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

#ifdef __cplusplus
}
#endif
