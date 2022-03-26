#include "Common.h"
#include <sys/stat.h>
#include <ctype.h>

//#define DX_USE_LOG_FILE

#define DX_MAX_LOADSTRING MAX_PATH

LPTSTR LoadStringDx(INT nID)
{
    static TCHAR s_szText[3][DX_MAX_LOADSTRING];
    static size_t s_i = 0;
    size_t i = s_i;
    s_szText[i][0] = 0;
    LoadString(NULL, nID, s_szText[i], _countof(s_szText[i]));
    s_i = (s_i + 1) % _countof(s_szText);
    return s_szText[i];
}

LONG RegMakeDx(HKEY hKey, LPCTSTR name, HKEY *phkeyResult)
{
    return RegCreateKeyEx(hKey, name, 0, NULL, 0, KEY_WRITE, NULL, phkeyResult, NULL);
}

LPSTR GetWindowTextDxA(HWND hwnd)
{
    INT cch = GetWindowTextLengthA(hwnd) + 1;
    LPSTR ret = (LPSTR)calloc(cch, sizeof(CHAR));
    if (!ret)
    {
        assert(0);
        return NULL;
    }
    GetWindowTextA(hwnd, ret, cch);
    return ret;
}

LPWSTR GetWindowTextDxW(HWND hwnd)
{
    INT cch = GetWindowTextLengthW(hwnd) + 1;
    LPWSTR ret = (LPWSTR)calloc(cch, sizeof(WCHAR));
    if (!ret)
    {
        assert(0);
        return NULL;
    }
    GetWindowTextW(hwnd, ret, cch);
    return ret;
}

SIZE SizeFromRectDx(LPCRECT prc)
{
    SIZE siz = { prc->right - prc->left, prc->bottom - prc->top };
    return siz;
}

RECT WorkAreaFromWindowDx(HWND hwnd)
{
    MONITORINFO mi = { sizeof(mi) };
    RECT rc;
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi))
    {
        return mi.rcWork;
    }
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    return rc;
}

VOID RepositionPointDx(LPPOINT ppt, SIZE siz, LPCRECT prc)
{
    if (ppt->x + siz.cx > prc->right)
        ppt->x = prc->right - siz.cx;
    if (ppt->y + siz.cy > prc->bottom)
        ppt->y = prc->bottom - siz.cy;
    if (ppt->x < prc->left)
        ppt->x = prc->left;
    if (ppt->y < prc->top)
        ppt->y = prc->top;
}

VOID RepositionWindowDx(HWND hwnd)
{
    RECT rc;
    SIZE siz;
    POINT pt;
    RECT rcWork = WorkAreaFromWindowDx(hwnd);
    GetWindowRect(hwnd, &rc);
    pt.x = rc.left;
    pt.y = rc.top;
    siz.cx = rc.right - rc.left;
    siz.cy = rc.bottom - rc.top;
    RepositionPointDx(&pt, siz, &rcWork);
    MoveWindow(hwnd, pt.x, pt.y, siz.cx, siz.cy, TRUE);
}

VOID CenterWindowDx(HWND hwnd)
{
    HWND hwndParent;
    BOOL bChild;
    RECT rc, rcWork, rcParent;
    SIZE siz, sizParent;
    POINT pt;

    assert(IsWindow(hwnd));

    bChild = !!(GetWindowStyle(hwnd) & WS_CHILD);
    if (bChild)
        hwndParent = GetParent(hwnd);
    else
        hwndParent = GetWindow(hwnd, GW_OWNER);

    rcWork = WorkAreaFromWindowDx(hwnd);

    if (hwndParent)
        GetWindowRect(hwndParent, &rcParent);
    else
        rcParent = rcWork;

    sizParent = SizeFromRectDx(&rcParent);

    GetWindowRect(hwnd, &rc);
    siz = SizeFromRectDx(&rc);

    pt.x = rcParent.left + (sizParent.cx - siz.cx) / 2;
    pt.y = rcParent.top + (sizParent.cy - siz.cy) / 2;

    if (bChild && hwndParent)
    {
        GetClientRect(hwndParent, &rcParent);
        MapWindowPoints(hwndParent, NULL, (LPPOINT)&rcParent, 2);
        RepositionPointDx(&pt, siz, &rcParent);

        ScreenToClient(hwndParent, &pt);
    }
    else
    {
        RepositionPointDx(&pt, siz, &rcWork);
    }

    SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static BOOL s_bMsgBoxHooked = FALSE;

static LRESULT CALLBACK MsgBoxCbtProcDx(INT nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_ACTIVATE)
    {
        HWND hwnd = (HWND)wParam;
        TCHAR szClassName[16];
        GetClassName(hwnd, szClassName, _countof(szClassName));
        if (lstrcmp(szClassName, TEXT("#32770")) == 0)
        {
            CenterWindowDx(hwnd);
            s_bMsgBoxHooked = FALSE;
        }
    }
    return 0;
}

static VOID HookMsgBoxDx(BOOL bHook)
{
    static HHOOK s_hHook = NULL;
    if (bHook)
    {
        if (s_hHook == NULL)
        {
            DWORD dwThreadID = GetCurrentThreadId();
            s_hHook = SetWindowsHookEx(WH_CBT, MsgBoxCbtProcDx, NULL, dwThreadID);
        }
        s_bMsgBoxHooked = TRUE;
    }
    else
    {
        s_bMsgBoxHooked = FALSE;
        if (s_hHook)
        {
            if (UnhookWindowsHookEx(s_hHook))
            {
                s_hHook = NULL;
            }
        }
    }
}

LPTSTR MakeFilterDx(LPTSTR psz)
{
    LPTSTR pch, pchNext;
    for (pch = psz; *pch; pch = pchNext)
    {
        pchNext = CharNext(pch);
        if (*pch == TEXT('|'))
            *pch = 0;
    }
    return psz;
}

INT MsgBoxDx(HWND hwnd, LPCTSTR text, LPCTSTR title, UINT uType)
{
    HookMsgBoxDx(TRUE);
    INT ret = MessageBox(hwnd, text, title, uType);
    HookMsgBoxDx(FALSE);
    return ret;
}

INT ErrorBoxDx(HWND hwnd, LPCTSTR text)
{
    return MsgBoxDx(hwnd, text, NULL, MB_ICONERROR);
}

void DebugPrintfDxA(const char *fmt, ...)
{
#ifdef DX_USE_LOG_FILE
    va_list va;
    FILE *fp = fopen("log.txt", "a");
    va_start(va, fmt);
    vfprintf(fp, fmt, va);
    fclose(fp);
    va_end(va);
#else
    char buf[MAX_PATH];
    va_list va;
    va_start(va, fmt);
    StringCchVPrintfA(buf, _countof(buf), fmt, va);
    OutputDebugStringA(buf);
    va_end(va);
#endif
}

void DebugPrintfDxW(const wchar_t *fmt, ...)
{
#ifdef DX_USE_LOG_FILE
    va_list va;
    FILE *fp = fopen("log.txt", "a");
    va_start(va, fmt);
    vfwprintf(fp, fmt, va);
    fclose(fp);
    va_end(va);
#else
    wchar_t buf[MAX_PATH];
    va_list va;
    va_start(va, fmt);
    StringCchVPrintfW(buf, _countof(buf), fmt, va);
    OutputDebugStringW(buf);
    va_end(va);
#endif
}

BOOL EnableProcessPrivilegeDx(LPCTSTR pszSE_)
{
    BOOL f = FALSE;
    LUID luid;
    TOKEN_PRIVILEGES tp;
    HANDLE hToken, hProcess = GetCurrentProcess();

    if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        if (LookupPrivilegeValue(NULL, pszSE_, &luid))
        {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            tp.Privileges[0].Luid = luid;
            f = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
        }
        CloseHandle(hToken);
    }

    return f;
}

INT WriteToFileDx(LPCTSTR filename, const void *pData, size_t size)
{
    FILE *fout;

    fout = _tfopen(filename, TEXT("wb"));
    if (!fout)
        return -1;

    if (fwrite(pData, 1, size, fout) != size)
    {
        fclose(fout);
        return -2;
    }

    fclose(fout);
    return 0;
}

INT ReadFromFileDx(LPCTSTR filename, void **ppData, size_t *psize)
{
    FILE *fin;
    size_t size, readLen;
    char *pData;
#ifdef _WIN32
    struct _stat st;
#else
    struct stat st;
#endif

    assert(psize != NULL);
    assert(ppData != NULL);

    *ppData = NULL;

    if (_tstat(filename, &st) != 0)
        return -1;

    *psize = size = st.st_size;

    fin = _tfopen(filename, TEXT("rb"));
    if (!fin)
        return -2;

    pData = malloc(size + 1); // including NUL
    if (!pData)
    {
        fclose(fin);
        return -3;
    }

    readLen = fread(pData, 1, size, fin);
    pData[size] = 0; // set NUL

    fclose(fin);

    if (readLen != size)
    {
        free(pData);
        return -4;
    }

    *ppData = pData;
    return 0; // success
}

void RebootDx(BOOL bForce)
{
    EnableProcessPrivilegeDx(SE_SHUTDOWN_NAME);
    ExitWindowsEx(EWX_REBOOT | (bForce ? EWX_FORCEIFHUNG : 0), 0);
}

void LogOffDx(BOOL bForce)
{
    EnableProcessPrivilegeDx(SE_SHUTDOWN_NAME);
    ExitWindowsEx(EWX_LOGOFF | (bForce ? EWX_FORCEIFHUNG : 0), 0);
}

void PowerOffDx(BOOL bForce)
{
    EnableProcessPrivilegeDx(SE_SHUTDOWN_NAME);
    ExitWindowsEx(EWX_POWEROFF | (bForce ? EWX_FORCEIFHUNG : 0), 0);
}

void ShutDownDx(BOOL bForce)
{
    EnableProcessPrivilegeDx(SE_SHUTDOWN_NAME);
    ExitWindowsEx(EWX_SHUTDOWN | (bForce ? EWX_FORCEIFHUNG : 0), 0);
}

DWORD GetComCtl32VersionDx(VOID)
{
    DLLGETVERSIONPROC fn;
    DLLVERSIONINFO dvi = { sizeof(dvi) };
    HMODULE hComCtl32 = GetModuleHandle(TEXT("comctl32"));
    fn = (DLLGETVERSIONPROC)GetProcAddress(hComCtl32, "DllGetVersion");
    if (!fn || FAILED(fn(&dvi)))
        return 0;
    return MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
}

static BOOL s_bAutoAsserting = FALSE;
static INT s_nAutoAssertFailCount = 0;

BOOL EnableAutoAssertDx(BOOL bEnable)
{
    INT nFailCount = 0;
    if (bEnable)
    {
        ASSERT(!s_bAutoAsserting);
        s_bAutoAsserting = TRUE;
        s_nAutoAssertFailCount = 0;
    }
    else
    {
        ASSERT(s_bAutoAsserting);
        s_bAutoAsserting = FALSE;
        nFailCount = s_nAutoAssertFailCount;
        s_nAutoAssertFailCount = 0;
    }
    return nFailCount;
}

void AssertDx(const char *text, const char *file, int line)
{
    char szMsg[512];
    INT id;

    StringCchPrintfA(szMsg, _countof(szMsg),
        "Assertion Failure!\n\n"
        "File: %s\n"
        "Line: %d\n"
        "Expression: %s\n\n"
        "Click [Abort] to terminate the application.\n"
        "Click [Retry] to debug the application.\n"
        "Click [Ignore] to ignore this message.",
        file, line, text);

    if (s_bAutoAsserting)
    {
        ++s_nAutoAssertFailCount;
        TRACEA("%s\n", szMsg);
        return;
    }

    id = MessageBoxA(NULL, szMsg, "ASSERT", MB_ICONERROR | MB_ABORTRETRYIGNORE);
    switch (id)
    {
    case IDABORT:
        ExitProcess(0xDEADFACE);
        break;
    case IDRETRY:
        DebugBreak();
        break;
    case IDIGNORE:
        break;
    }
}

void HexDumpDx(const void *ptr, size_t size, size_t top_addr)
{
    const BYTE *pb = ptr;
    size_t ib, column, work, addr;
    static INT s_i = 0;

    ++s_i;

    TRACEA("---[HexDump-%d.txt]---(FROM HERE)---\n", s_i);

    if (!ptr)
    {
        TRACEA("(null)\n");
        goto Quit;
    }

    if (!size)
    {
        TRACEA("(Empty)\n");
        goto Quit;
    }

#ifdef _WIN64
    TRACEA("_ADDRESS:_ADDRESS");
#else
    TRACEA("_ADDRESS");
#endif
    TRACEA("  +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F  0123456789ABCDEF\n");
    for (ib = 0; ib < size; ib += 16)
    {
        addr = top_addr + ib;
#ifdef _WIN64
        TRACEA("%08X:%08X ", (DWORD)addr, (DWORD)(addr >> 32));
#else
        TRACEA("%08X ", addr);
#endif
        if (ib + 16 > size)
            work = size - ib;
        else
            work = 16;

        for (column = 0; column < 16; ++column)
        {
            if (work < column)
                TRACEA("   ");
            else
                TRACEA(" %02X", pb[ib + column]);
            if (column == 7)
                TRACEA(" ");
        }

        TRACEA("  ");
        for (column = 0; column < 16; ++column)
        {
            if (work < column)
                TRACEA(" ");
            else if (isprint(pb[ib + column]))
                TRACEA("%c", pb[ib + column]);
            else
                TRACEA(".");
        }

        TRACEA("\n");
    }

    TRACEA("Total: %zu bytes (0x%zX)\n", size, size);

Quit:
    TRACEA("---[HexDump-%d.txt]---(DOWN TO HERE)---\n", s_i);
}

SIZE GetTextExtentDx(LPCTSTR pszText, HFONT hFont)
{
    SIZE ret;
    HDC hDC = CreateCompatibleDC(NULL);
    HGDIOBJ hFontOld = SelectObject(hDC, hFont);
    GetTextExtentPoint32(hDC, pszText, lstrlen(pszText), &ret);
    SelectObject(hDC, hFontOld);
    DeleteDC(hDC);
    return ret;
}
