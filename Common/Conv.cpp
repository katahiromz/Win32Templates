#include "Common.h"

#define DX_MAX_CONV_COUNT  3
#define DX_MAX_CONV_BUFF   MAX_PATH

LPWSTR WideFromAnsi(LPCSTR pszAnsi, UINT nCodePage)
{
    INT cchWide = MultiByteToWideChar(nCodePage, 0, pszAnsi, -1, NULL, 0);
    LPWSTR pszWide = (LPWSTR)calloc(cchWide, sizeof(WCHAR));
    MultiByteToWideChar(nCodePage, 0, pszAnsi, -1, pszWide, cchWide);
    return pszWide;
}

LPSTR AnsiFromWide(LPCWSTR pszWide, UINT nCodePage)
{
    INT cchAnsi = WideCharToMultiByte(nCodePage, 0, pszWide, -1, NULL, 0, NULL, NULL);
    LPSTR pszAnsi = (LPSTR)calloc(cchAnsi, sizeof(char));
    WideCharToMultiByte(nCodePage, 0, pszWide, -1, pszAnsi, cchAnsi, NULL, NULL);
    return pszAnsi;
}

LPWSTR A2W(LPCSTR psz, UINT nCodePage)
{
    static WCHAR s_asz[DX_MAX_CONV_COUNT][DX_MAX_CONV_BUFF];
    static INT s_i = 0;
    INT i = s_i;
    s_asz[i][0] = 0;
    MultiByteToWideChar(nCodePage, 0, psz, -1, s_asz[i], DX_MAX_CONV_BUFF);
    s_i = (s_i + 1) % _countof(s_asz);
    return s_asz[i];
}

LPSTR W2A(LPCWSTR psz, UINT nCodePage)
{
    static CHAR s_asz[DX_MAX_CONV_COUNT][DX_MAX_CONV_BUFF];
    static INT s_i = 0;
    INT i = s_i;
    s_asz[i][0] = 0;
    WideCharToMultiByte(nCodePage, 0, psz, -1, s_asz[i], DX_MAX_CONV_BUFF, NULL, NULL);
    s_i = (s_i + 1) % _countof(s_asz);
    return s_asz[i];
}
