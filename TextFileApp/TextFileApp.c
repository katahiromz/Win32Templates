#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////
// GLOBAL

#define MAINWND_CLASSNAME   TEXT("WindowApp by katahiromz")

// The window IDs
#define IDW_CANVAS edt1
#define IDW_TOOLBAR ctl1
#define IDW_STATUSBAR stc1

HINSTANCE   g_hInstance       = NULL; // module handle
HWND        g_hMainWnd        = NULL; // main window
HWND        g_hCanvasWnd      = NULL; // IDW_CANVAS
HWND        g_hToolbar        = NULL; // IDW_TOOLBAR
HIMAGELIST  g_himlToolbar     = NULL; // image list for toolbar
HWND        g_hStatusBar      = NULL; // IDW_STATUSBAR
HACCEL      g_hAccel          = NULL; // IDR_ACCEL
BOOL        g_bFileModified   = FALSE; // Is file modified?
TCHAR g_szFileName[MAX_PATH]  = TEXT(""); // The full pathname of the open file
TCHAR g_szFileTitle[MAX_PATH] = TEXT(""); // The title name of the open file

///////////////////////////////////////////////////////////////////////////////
// SETTINGS

#define COMPANY_NAME  TEXT("Katayama Hirofumi MZ")
#define APP_NAME      TEXT("TextFileApp")
#define MAX_RECENTS   20

typedef struct SETTINGS
{
    INT nWindowX;
    INT nWindowY;
    INT nWindowCX;
    INT nWindowCY;
    BOOL bShowToolbar;
    BOOL bShowStatusBar;
    BOOL bMaximized;
    PRECENT pRecent;
} SETTINGS, *PSETTINGS;

SETTINGS g_settings;

BOOL loadDword(HKEY hAppKey, LPCTSTR name, LPDWORD pdwValue)
{
    DWORD cbValue = sizeof(DWORD);
    LONG error = RegQueryValueEx(hAppKey, name, NULL, NULL, (LPBYTE)pdwValue, &cbValue);
    return !error;
}

BOOL loadSz(HKEY hAppKey, LPCTSTR name, LPTSTR psz, DWORD cch, LPCTSTR pszDefValue)
{
    DWORD cbValue = (cch + 1) * sizeof(TCHAR);
    LONG error = RegQueryValueEx(hAppKey, name, NULL, NULL, (LPBYTE)psz, &cbValue);
    if (error)
        StringCchCopy(psz, cch, pszDefValue);
    return !error;
}

BOOL saveDword(HKEY hAppKey, LPCTSTR name, DWORD dwValue)
{
    DWORD cbValue = sizeof(dwValue);
    LONG error = RegSetValueEx(hAppKey, name, 0, REG_DWORD, (LPBYTE)&dwValue, cbValue);
    return !error;
}

BOOL saveSz(HKEY hAppKey, LPCTSTR name, LPCTSTR value)
{
    DWORD cbValue = (lstrlen(value) + 1) * sizeof(TCHAR);
    LONG error = RegSetValueEx(hAppKey, name, 0, REG_SZ, (LPBYTE)value, cbValue);
    return !error;
}

BOOL loadSettings(PSETTINGS pSettings)
{
    HKEY hAppKey = NULL;
    TCHAR szName[MAX_PATH], szText[MAX_PATH];
    INT i, nRecentCount;

    // initial values
    pSettings->nWindowX = CW_USEDEFAULT;
    pSettings->nWindowY = CW_USEDEFAULT;
    pSettings->nWindowCX = 600;
    pSettings->nWindowCY = 400;
    pSettings->bShowToolbar = TRUE;
    pSettings->bShowStatusBar = TRUE;
    pSettings->bMaximized = FALSE;
    pSettings->pRecent = Recent_New(MAX_RECENTS);

    RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\") COMPANY_NAME TEXT("\\") APP_NAME,
                 0, KEY_READ, &hAppKey);
    if (!hAppKey)
        return FALSE;

#define LOAD_DWORD(name, value) do { \
    assert(sizeof(value) == sizeof(DWORD)); \
    loadDword(hAppKey, TEXT(name), (LPDWORD)&value); \
} while (0)
    LOAD_DWORD("WindowX", pSettings->nWindowX);
    LOAD_DWORD("WindowY", pSettings->nWindowY);
    LOAD_DWORD("WindowCX", pSettings->nWindowCX);
    LOAD_DWORD("WindowCY", pSettings->nWindowCY);
    LOAD_DWORD("ShowToolbar", pSettings->bShowToolbar);
    LOAD_DWORD("ShowStatusBar", pSettings->bShowStatusBar);
    LOAD_DWORD("Maximized", pSettings->bMaximized);
    LOAD_DWORD("RecentCount", nRecentCount);
#undef LOAD_DWORD

    if (nRecentCount > MAX_RECENTS)
        nRecentCount = MAX_RECENTS;

    for (i = 0; i < nRecentCount; ++i)
    {
        StringCchPrintf(szName, _countof(szName), TEXT("Recent%u"), i);
        if (loadSz(hAppKey, szName, szText, _countof(szText), TEXT("")) && szText[0])
        {
            Recent_Add(pSettings->pRecent, szText);
        }
        else
        {
            break;
        }
    }

    RegCloseKey(hAppKey);
    return TRUE;
}

BOOL saveSettings(const SETTINGS *pSettings)
{
    HKEY hSoftwareKey = NULL, hCompanyKey = NULL, hAppKey = NULL;
    BOOL ret = FALSE;
    TCHAR szName[MAX_PATH];
    INT i, nRecentCount = Recent_GetCount(pSettings->pRecent);

    RegMakeDx(HKEY_CURRENT_USER, TEXT("Software"), &hSoftwareKey);
    if (!hSoftwareKey)
        goto Quit;
    RegMakeDx(hSoftwareKey, COMPANY_NAME, &hCompanyKey);
    if (!hCompanyKey)
        goto Quit;
    RegMakeDx(hCompanyKey, APP_NAME, &hAppKey);
    if (!hAppKey)
        goto Quit;

#define SAVE_DWORD(name, value) do { \
    saveDword(hAppKey, TEXT(name), (value)); \
} while (0)
    SAVE_DWORD("WindowX", pSettings->nWindowX);
    SAVE_DWORD("WindowY", pSettings->nWindowY);
    SAVE_DWORD("WindowCX", pSettings->nWindowCX);
    SAVE_DWORD("WindowCY", pSettings->nWindowCY);
    SAVE_DWORD("ShowToolbar", pSettings->bShowToolbar);
    SAVE_DWORD("ShowStatusBar", pSettings->bShowStatusBar);
    SAVE_DWORD("Maximized", pSettings->bMaximized);
    SAVE_DWORD("RecentCount", nRecentCount);
#undef SAVE_DWORD

    for (i = 0; i < nRecentCount; ++i)
    {
        StringCchPrintf(szName, _countof(szName), TEXT("Recent%u"), i);
        saveSz(hAppKey, szName, Recent_GetAt(pSettings->pRecent, i));
    }

    ret = TRUE;

Quit:
    RegCloseKey(hAppKey);
    RegCloseKey(hCompanyKey);
    RegCloseKey(hSoftwareKey);
    return ret;
}

void eraseRecent(LPCTSTR pszFile)
{
    Recent_Remove(g_settings.pRecent, pszFile);
}

void addRecent(LPCTSTR pszFile)
{
    Recent_Add(g_settings.pRecent, pszFile);
}

///////////////////////////////////////////////////////////////////////////////
// LOADING/SAVING FILES

BOOL doSave(HWND hwnd);

void updateModified(BOOL bModified)
{
    g_bFileModified = bModified;
}

void updateFileName(HWND hwnd, LPCTSTR pszFile)
{
    TCHAR szTitle[MAX_PATH];
    LPTSTR pchTitle;

    if (!pszFile || !pszFile[0])
    {
        g_szFileName[0] = g_szFileTitle[0] = 0;
        SetWindowText(hwnd, LoadStringDx(IDS_APPNAME));
        return;
    }

    GetFullPathName(pszFile, _countof(g_szFileName), g_szFileName, &pchTitle);
    StringCchCopy(g_szFileTitle, _countof(g_szFileTitle), pchTitle);

    StringCchPrintf(szTitle, _countof(szTitle), LoadStringDx(IDS_TITLE), pchTitle);
    SetWindowText(hwnd, szTitle);

    addRecent(g_szFileName);
}

BOOL checkFileChange(HWND hwnd)
{
    INT id;

    if (!g_bFileModified)
        return TRUE;

    id = MsgBoxDx(hwnd, LoadStringDx(IDS_WANNASAVE), LoadStringDx(IDS_APPNAME),
                  MB_ICONINFORMATION | MB_YESNOCANCEL);
    switch (id)
    {   
    case IDYES:
        if (doSave(hwnd))
            return TRUE;
        break;
    case IDNO:
        return TRUE;
    case IDCANCEL:
        break;
    }

    return FALSE;
}

BOOL doLoadFile(HWND hwnd, LPCTSTR pszFile)
{
    void *pData;
    size_t size;
    INT err;
    TCHAR szFilePath[MAX_PATH];

    if (lstrcmpi(PathFindExtension(pszFile), TEXT(".LNK")) == 0) // shortcut file
    {
        if (!GetPathOfShortcutDx(hwnd, pszFile, szFilePath))
        {
            ErrorBoxDx(hwnd, LoadStringDx(IDS_READERROR));
            return FALSE;
        }
        pszFile = szFilePath;
    }

    err = ReadFromFileDx(pszFile, &pData, &size);
    if (err < 0)
    {
        ErrorBoxDx(hwnd, LoadStringDx(IDS_READERROR));
        return FALSE;
    }

    SetWindowTextA(g_hCanvasWnd, (LPCSTR)pData);
    updateFileName(hwnd, pszFile);
    updateModified(FALSE);
    return TRUE;
}

BOOL doSaveFile(HWND hwnd, LPCTSTR pszFile)
{
    LPTSTR psz;
    INT err;

    psz = GetWindowTextDx(g_hCanvasWnd);
    if (!psz)
    {
        ErrorBoxDx(hwnd, LoadStringDx(IDS_WRITEERROR));
        return FALSE;
    }

    err = WriteToFileDx(pszFile, psz, lstrlen(psz));
    if (err < 0)
    {
        ErrorBoxDx(hwnd, LoadStringDx(IDS_WRITEERROR));
        return FALSE;
    }

    updateFileName(hwnd, pszFile);
    updateModified(FALSE);
    return TRUE;
}

BOOL doNew(HWND hwnd)
{
    if (!checkFileChange(hwnd))
        return FALSE;
    SetWindowText(g_hCanvasWnd, NULL);
    updateFileName(hwnd, NULL);
    updateModified(FALSE);
    return TRUE;
}

BOOL doOpen(HWND hwnd)
{
    TCHAR szFile[MAX_PATH] = TEXT("");
    OPENFILENAME ofn = { sizeof(ofn), hwnd };

    if (!checkFileChange(hwnd))
        return FALSE;

    ofn.lpstrFilter = MakeFilterDx(LoadStringDx(IDS_FILTER));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrTitle = LoadStringDx(IDS_OPEN);
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("txt");
    if (GetOpenFileName(&ofn))
    {
        return doLoadFile(hwnd, szFile);
    }
    return FALSE;
}

BOOL doSaveAs(HWND hwnd)
{
    TCHAR szFile[MAX_PATH] = TEXT("");
    OPENFILENAME ofn = { sizeof(ofn), hwnd };
    if (g_szFileName)
    {
        StringCchCopy(szFile, _countof(szFile), g_szFileName);
    }
    ofn.lpstrFilter = MakeFilterDx(LoadStringDx(IDS_FILTER));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrTitle = LoadStringDx(IDS_SAVEAS);
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = TEXT("txt");
    if (GetSaveFileName(&ofn))
    {
        return doSaveFile(hwnd, szFile);
    }
    return FALSE;
}

BOOL doSave(HWND hwnd)
{
    if (g_szFileName[0])
        return doSaveFile(hwnd, g_szFileName);
    else
        return doSaveAs(hwnd);
}

BOOL doParseCommandLine(HWND hwnd, INT argc, LPTSTR *argv)
{
    for (INT i = 1; i < argc; ++i)
    {
        if (!doLoadFile(hwnd, argv[i]))
            return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CONTROLS

BOOL doCreateToolbar(HWND hwnd)
{
    DWORD style, exstyle;
    INT id;

    // TODO: Extend toolbar buttons
    static const TBBUTTON buttons[] =
    {
        { 0, ID_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON },
        { -1, -1, TBSTATE_ENABLED, TBSTYLE_SEP },
        { 1, ID_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON },
        { 2, ID_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON },
    };
    INT nButtonImageWidth = 32; // TODO: Change
    INT nButtonImageHeight = 32; // TODO: Change
    INT nButtonWidth = 36; // TODO: Change
    INT nButtonHeight = 36; // TODO: Change
    COLORREF rgbMaskColor = RGB(255, 0, 255); // TODO: Change
    g_himlToolbar = ImageList_LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR),
                                         nButtonImageWidth, 0, rgbMaskColor);
    if (!g_himlToolbar)
        return FALSE;

    style = WS_CHILD | TBS_HORZ | TBS_TOOLTIPS;
    if (g_settings.bShowToolbar)
        style |= WS_VISIBLE;
    exstyle = 0;
    id = IDW_TOOLBAR;
    g_hToolbar = CreateWindowEx(exstyle, TOOLBARCLASSNAME, NULL,
                                style, 0, 0, 0, 0, hwnd,
                                (HMENU)(INT_PTR)IDW_TOOLBAR, g_hInstance, NULL);
    if (!g_hToolbar)
        return FALSE;

    SendMessage(g_hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(g_hToolbar, TB_SETIMAGELIST, 0, (LPARAM)g_himlToolbar);
    SendMessage(g_hToolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(nButtonImageWidth, nButtonImageHeight));
    SendMessage(g_hToolbar, TB_SETBUTTONSIZE, 0, MAKELPARAM(nButtonWidth, nButtonHeight));
    SendMessage(g_hToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);
    return TRUE;
}

BOOL createControls(HWND hwnd)
{
    DWORD style, exstyle;
    INT id;

    style = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL;
    exstyle = WS_EX_CLIENTEDGE;
    id = IDW_CANVAS;
    g_hCanvasWnd = CreateWindowEx(exstyle, TEXT("EDIT"), NULL, style, 0, 0, 0, 0,
        hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!g_hCanvasWnd)
        return FALSE;
    SetWindowFont(g_hCanvasWnd, GetStockFont(DEFAULT_GUI_FONT), TRUE);

    if (!doCreateToolbar(hwnd))
        return FALSE;

    style = WS_CHILD | SBS_SIZEGRIP;
    if (g_settings.bShowStatusBar)
        style |= WS_VISIBLE;
    exstyle = 0;
    id = IDW_STATUSBAR;
    g_hStatusBar = CreateStatusWindow(style, NULL, hwnd, id);
    if (!g_hStatusBar)
        return FALSE;

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// MESSAGE HANDLERS

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    //CenterWindowDx(hwnd);
    DragAcceptFiles(hwnd, TRUE);

    g_hAccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL));
    if (!g_hAccel)
        return FALSE;

    if (!createControls(hwnd))
        return FALSE;

#ifdef UNICODE
    {
        INT wargc;
        LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargv);
        if (!doParseCommandLine(hwnd, wargc, wargv))
        {
            LocalFree(wargv);
            return FALSE;
        }
        LocalFree(wargv);
    }
#else
    if (!doParseCommandLine(hwnd, __argc, __argv))
    {
        return FALSE;
    }
#endif

    PostMessage(hwnd, WM_SIZE, 0, 0);
    PostMessage(hwnd, WM_COMMAND, 0, 0);
    return TRUE;
}

void OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
    SetFocus(g_hCanvasWnd);
}

void OnDropFiles(HWND hwnd, HDROP hdrop)
{
    TCHAR szFile[MAX_PATH];
    DragQueryFile(hdrop, 0, szFile, _countof(szFile));
    doLoadFile(hwnd, szFile);
    DragFinish(hdrop);
}

BOOL about_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    CenterWindowDx(hwnd);
    return TRUE;
}

void about_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    }
}

INT_PTR CALLBACK
aboutDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, about_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, about_OnCommand);
    }
    return 0;
}

BOOL doTest(HWND hwnd)
{
    Recent_UnitTest();
    MsgBoxDx(hwnd, TEXT("This is a test"), LoadStringDx(IDS_APPNAME), MB_ICONINFORMATION);
    return TRUE;
}

void OnRecent(HWND hwnd, INT index)
{
    LPCTSTR psz = Recent_GetAt(g_settings.pRecent, index);
    if (psz)
        doLoadFile(hwnd, psz);
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    static INT s_nBusyLock = 0;

    if (s_nBusyLock == 0 && IsWindowVisible(g_hStatusBar))
    {
        SendMessage(g_hStatusBar, SB_SETTEXT, 0 | 0, (LPARAM)LoadStringDx(IDS_BUSY));
    }

    ++s_nBusyLock;

    switch (id)
    {
    case ID_EXIT:
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        break;
    case ID_ABOUT:
        DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, aboutDialogProc);
        break;
    case ID_TEST:
        doTest(hwnd);
        break;
    case ID_NEW:
        doNew(hwnd);
        break;
    case ID_OPEN:
        doOpen(hwnd);
        break;
    case ID_SAVE:
        doSave(hwnd);
        break;
    case ID_SAVEAS:
        doSaveAs(hwnd);
        break;
    case ID_STATUSBAR:
        if (IsWindowVisible(g_hStatusBar))
        {
            ShowWindow(g_hStatusBar, SW_HIDE);
        }
        else
        {
            ShowWindow(g_hStatusBar, SW_SHOWNOACTIVATE);
        }
        g_settings.bShowStatusBar = IsWindowVisible(g_hStatusBar);
        PostMessage(hwnd, WM_SIZE, 0, 0);
        break;
    case ID_TOOLBAR:
        if (IsWindowVisible(g_hToolbar))
        {
            ShowWindow(g_hToolbar, SW_HIDE);
        }
        else
        {
            ShowWindow(g_hToolbar, SW_SHOWNOACTIVATE);
        }
        g_settings.bShowToolbar = IsWindowVisible(g_hToolbar);
        PostMessage(hwnd, WM_SIZE, 0, 0);
        break;
    case IDW_CANVAS:
        if (codeNotify == EN_CHANGE)
        {
            updateModified(TRUE);
        }
        break;
    default:
        if (ID_RECENT_01 <= id && id <= ID_RECENT_20)
        {
            OnRecent(hwnd, id - ID_RECENT_01);
        }
        break;
    }

    --s_nBusyLock;

    if (s_nBusyLock == 0 && IsWindowVisible(g_hStatusBar))
    {
        SendMessage(g_hStatusBar, SB_SETTEXT, 0 | 0, (LPARAM)LoadStringDx(IDS_READY));
    }
}

BOOL OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    FillRect(hdc, &rc, GetStockBrush(GRAY_BRUSH));
    return TRUE;
}

void drawClient(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    MoveToEx(hdc, rc.left, rc.top, NULL);
    LineTo(hdc, rc.right, rc.bottom);

    MoveToEx(hdc, rc.right, rc.top, NULL);
    LineTo(hdc, rc.left, rc.bottom);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    if (hdc)
    {
        drawClient(hwnd, hdc);
        EndPaint(hwnd, &ps);
    }
}

void OnMove(HWND hwnd, int x, int y)
{
    RECT rcWnd;
    GetWindowRect(hwnd, &rcWnd);

    if (g_hMainWnd && !IsIconic(hwnd) && !IsZoomed(hwnd))
    {
        g_settings.nWindowX = rcWnd.left;
        g_settings.nWindowY = rcWnd.top;
    }
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    RECT rc, rcWnd, rcStatus, rcToolbar;

    GetClientRect(hwnd, &rc);
    GetWindowRect(hwnd, &rcWnd);

    if (g_hMainWnd)
    {
        if (!IsIconic(hwnd) && !IsZoomed(hwnd))
        {
            g_settings.nWindowCX = rcWnd.right - rcWnd.left;
            g_settings.nWindowCY = rcWnd.bottom - rcWnd.top;
        }
        g_settings.bMaximized = IsZoomed(hwnd);
    }

    SendMessage(g_hToolbar, TB_AUTOSIZE, 0, 0);
    SendMessage(g_hStatusBar, WM_SIZE, 0, 0);

    if (IsWindowVisible(g_hToolbar))
    {
        GetWindowRect(g_hToolbar, &rcToolbar);

        rc.top += rcToolbar.bottom - rcToolbar.top;
    }

    if (IsWindowVisible(g_hStatusBar))
    {
        INT an[2];
        GetWindowRect(g_hStatusBar, &rcStatus);

        an[0] = rcStatus.right - rcStatus.left - 100;
        an[1] = -1;
        SendMessage(g_hStatusBar, SB_SETPARTS, 2, (LPARAM)an);

        rc.bottom -= rcStatus.bottom - rcStatus.top;
    }

    MoveWindow(g_hCanvasWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void OnClose(HWND hwnd)
{
    if (checkFileChange(hwnd))
    {
        DestroyWindow(hwnd);
    }
}

void OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}

void OnInitMenu(HWND hwnd, HMENU hMenu)
{
    INT i, nRecentCount = Recent_GetCount(g_settings.pRecent);
    HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
    INT nFileMenuCount = GetMenuItemCount(hFileMenu);
    HMENU hRecentMenu = GetSubMenu(hFileMenu, nFileMenuCount - 3);
    LPCTSTR pszRecent;
    TCHAR szText[MAX_PATH];

    while (DeleteMenu(hRecentMenu, 0, MF_BYPOSITION))
        ;

    for (i = 0; i < nRecentCount; ++i)
    {
        pszRecent = Recent_GetAt(g_settings.pRecent, i);
        if (!pszRecent)
            continue;

        StringCchPrintf(szText, _countof(szText), TEXT("&%c   %s"),
            TEXT("123456789ABCDEFGHIJK")[i], PathFindFileName(pszRecent));
        AppendMenu(hRecentMenu, MF_STRING | MF_ENABLED, ID_RECENT_01 + i, szText);
    }

    if (Recent_GetCount(g_settings.pRecent) == 0)
    {
        AppendMenu(hRecentMenu, MF_STRING | MF_GRAYED, 0, LoadStringDx(IDS_NONE));
    }

    if (IsWindowVisible(g_hToolbar))
        CheckMenuItem(hMenu, ID_TOOLBAR, MF_CHECKED);
    else
        CheckMenuItem(hMenu, ID_TOOLBAR, MF_UNCHECKED);

    if (IsWindowVisible(g_hStatusBar))
        CheckMenuItem(hMenu, ID_STATUSBAR, MF_CHECKED);
    else
        CheckMenuItem(hMenu, ID_STATUSBAR, MF_UNCHECKED);
}

///////////////////////////////////////////////////////////////////////////////
// APPLICATION FRAMEWORK

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_ACTIVATE, OnActivate);
        HANDLE_MSG(hwnd, WM_DROPFILES, OnDropFiles);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_MOVE, OnMove);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(hwnd, WM_INITMENU, OnInitMenu);
        HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL doInitApp(HINSTANCE hInstance, LPSTR lpCmdLine)
{
    WNDCLASSEX wcx = { sizeof(wcx), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS };
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = hInstance;
    //wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    //wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcx.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcx.lpszClassName = MAINWND_CLASSNAME;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx))
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILREGCLASS), NULL, MB_ICONERROR);
        return FALSE;
    }
    return TRUE;
}

BOOL doCreateMainWnd(HINSTANCE hInstance, INT nCmdShow)
{
    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    DWORD exstyle = 0;
    INT x = g_settings.nWindowX, y = g_settings.nWindowY;
    INT cx = g_settings.nWindowCX, cy = g_settings.nWindowCY;
    HWND hwnd = CreateWindowEx(exstyle, MAINWND_CLASSNAME, LoadStringDx(IDS_APPNAME), style,
                               x, y, cx, cy, NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILCREATEWND), NULL, MB_ICONERROR);
        return FALSE;
    }

    RepositionWindowDx(hwnd);

    if (g_settings.bMaximized)
        ShowWindow(hwnd, SW_MAXIMIZE);
    else
        ShowWindow(hwnd, nCmdShow);

    UpdateWindow(hwnd);
    g_hMainWnd = hwnd;
    return TRUE;
}

INT doMainLoop(void)
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (g_hMainWnd && TranslateAccelerator(g_hMainWnd, g_hAccel, &msg))
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (INT)msg.wParam;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    INT ret;
    g_hInstance = hInstance;
    InitCommonControls();

    loadSettings(&g_settings);

    if (!doInitApp(hInstance, lpCmdLine))
        return -1;
    if (!doCreateMainWnd(hInstance, nCmdShow))
        return -1;

    ret = doMainLoop();

    saveSettings(&g_settings);
    Recent_Delete(g_settings.pRecent);

#if (WINVER >= 0x0500) && !defined(NDEBUG) && 1
    // for detecting object leak (Windows only)
    {
        HANDLE hProcess = GetCurrentProcess();
        DebugPrintfA("Count of GDI objects: %ld\n", GetGuiResources(hProcess, GR_GDIOBJECTS));
        DebugPrintfA("Count of USER objects: %ld\n", GetGuiResources(hProcess, GR_USEROBJECTS));
    }
#endif

#if defined(_MSC_VER) && !defined(NDEBUG) && 1
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}
