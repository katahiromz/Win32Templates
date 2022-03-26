#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////
// GLOBAL

HINSTANCE   g_hInstance       = NULL; // module handle
HWND        g_hMainWnd        = NULL; // main window
HWND        g_hCanvasWnd      = NULL; // IDW_CANVAS
HWND        g_hToolbar        = NULL; // IDW_TOOLBAR
HWND        g_hStatusBar      = NULL; // IDW_STATUSBAR
HACCEL      g_hAccel          = NULL; // IDR_ACCEL
BOOL        g_bFileModified   = FALSE; // Is file modified?
TCHAR g_szFileName[MAX_PATH]  = TEXT(""); // The full pathname of the open file
TCHAR g_szFileTitle[MAX_PATH] = TEXT(""); // The title name of the open file

// CommandUI.c
void dumpCommandUI(void);
LPTSTR getCommandText(INT id, BOOL bDetail);
void updateCommandUI(HWND hwnd, HMENU hMenu);
BOOL registerControls(HINSTANCE hInst);
BOOL createControls(HWND hwnd);
void destroyControls(HWND hwnd);
void OnMenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam);

// AboutDlg.c
void doAboutDlg(HWND hwnd);

///////////////////////////////////////////////////////////////////////////////
// PROFILE

typedef struct PROFILE
{
    INT nWindowX;
    INT nWindowY;
    INT nWindowCX;
    INT nWindowCY;
    BOOL bShowToolbar;
    BOOL bShowStatusBar;
    BOOL bMaximized;
    PRECENT pRecent;
} PROFILE, *PPROFILE;

PROFILE g_profile;

BOOL loadProfile(PPROFILE pProfile, INT nMaxRecents)
{
    doSetRegistryKey(DX_APP_COMPANY_NAME_IN_ENGLISH);

#define LOAD_INT(section, name, var, defvalue) do { \
    (var) = loadProfileInt(TEXT(section), TEXT(name), defvalue); \
} while (0)
    LOAD_INT("Settings", "WindowX", pProfile->nWindowX, CW_USEDEFAULT);
    LOAD_INT("Settings", "WindowY", pProfile->nWindowY, CW_USEDEFAULT);
    LOAD_INT("Settings", "WindowCX", pProfile->nWindowCX, 600);
    LOAD_INT("Settings", "WindowCY", pProfile->nWindowCY, 400);
    LOAD_INT("Settings", "ShowToolbar", pProfile->bShowToolbar, TRUE);
    LOAD_INT("Settings", "ShowStatusBar", pProfile->bShowStatusBar, TRUE);
    LOAD_INT("Settings", "Maximized", pProfile->bMaximized, FALSE);
#undef LOAD_INT

    pProfile->pRecent = loadRecentFileList(nMaxRecents, NULL);
    return TRUE;
}

BOOL saveProfile(const PROFILE *pProfile)
{
#define SAVE_INT(section, name, value) do { \
    saveProfileInt(TEXT(section), TEXT(name), (value)); \
} while (0)
    SAVE_INT("Settings", "WindowX", pProfile->nWindowX);
    SAVE_INT("Settings", "WindowY", pProfile->nWindowY);
    SAVE_INT("Settings", "WindowCX", pProfile->nWindowCX);
    SAVE_INT("Settings", "WindowCY", pProfile->nWindowCY);
    SAVE_INT("Settings", "ShowToolbar", pProfile->bShowToolbar);
    SAVE_INT("Settings", "ShowStatusBar", pProfile->bShowStatusBar);
    SAVE_INT("Settings", "Maximized", pProfile->bMaximized);
#undef SAVE_INT

    saveRecentFileList(pProfile->pRecent, NULL);
    return TRUE;
}

void doAddToRecentFileList(LPCTSTR pszFile)
{
    Recent_Add(g_profile.pRecent, pszFile);
}

///////////////////////////////////////////////////////////////////////////////
// LOADING/SAVING FILES

BOOL OnFileSave(HWND hwnd);

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

    doAddToRecentFileList(g_szFileName);
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
        if (OnFileSave(hwnd))
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
    free(pData);

    updateFileName(hwnd, pszFile);
    updateModified(FALSE);
    return TRUE;
}

BOOL doSaveFile(HWND hwnd, LPCTSTR pszFile)
{
    LPSTR psz;
    INT err;

    psz = GetWindowTextDxA(g_hCanvasWnd);
    if (!psz)
    {
        ErrorBoxDx(hwnd, LoadStringDx(IDS_WRITEERROR));
        return FALSE;
    }

    err = WriteToFileDx(pszFile, psz, lstrlenA(psz));
    free(psz);

    if (err < 0)
    {
        ErrorBoxDx(hwnd, LoadStringDx(IDS_WRITEERROR));
        return FALSE;
    }

    updateFileName(hwnd, pszFile);
    updateModified(FALSE);
    return TRUE;
}

BOOL OnFileNew(HWND hwnd)
{
    if (!checkFileChange(hwnd))
        return FALSE;
    SetWindowText(g_hCanvasWnd, NULL);
    updateFileName(hwnd, NULL);
    updateModified(FALSE);
    return TRUE;
}

BOOL OnFileOpen(HWND hwnd)
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

BOOL OnFileSaveAs(HWND hwnd)
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

BOOL OnFileSave(HWND hwnd)
{
    if (g_szFileName[0])
        return doSaveFile(hwnd, g_szFileName);
    else
        return OnFileSaveAs(hwnd);
}

BOOL doParseCommandLineEx(INT argc, LPTSTR *argv)
{
    HWND hwnd = g_hMainWnd;

    for (INT i = 1; i < argc; ++i)
    {
        if (!doLoadFile(hwnd, argv[i]))
            return FALSE;
    }

    return TRUE;
}

BOOL doParseCommandLine(void)
{
#ifdef UNICODE
    {
        INT wargc;
        LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if (!doParseCommandLineEx(wargc, wargv))
        {
            LocalFree(wargv);
            return FALSE;
        }
        LocalFree(wargv);
    }
#else
    if (!doParseCommandLineEx(__argc, __argv))
    {
        return FALSE;
    }
#endif
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

    if (g_profile.bShowToolbar)
        ShowWindow(g_hToolbar, SW_SHOWNOACTIVATE);
    if (g_profile.bShowStatusBar)
        ShowWindow(g_hStatusBar, SW_SHOWNOACTIVATE);

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

BOOL doTest(HWND hwnd)
{
    Recent_UnitTest();
    dumpCommandUI();
    HexDumpDx(g_hInstance, 343, (size_t)g_hInstance);
    MsgBoxDx(hwnd, TEXT("This is a test"), LoadStringDx(IDS_APPNAME), MB_ICONINFORMATION);
    return TRUE;
}

void OnRecent(HWND hwnd, INT index)
{
    LPCTSTR psz = Recent_GetAt(g_profile.pRecent, index);
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
        doAboutDlg(hwnd);
        break;
    case ID_TEST:
        doTest(hwnd);
        break;
    case ID_NEW:
        OnFileNew(hwnd);
        break;
    case ID_OPEN:
        OnFileOpen(hwnd);
        break;
    case ID_SAVE:
        OnFileSave(hwnd);
        break;
    case ID_SAVEAS:
        OnFileSaveAs(hwnd);
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
        g_profile.bShowStatusBar = IsWindowVisible(g_hStatusBar);
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
        g_profile.bShowToolbar = IsWindowVisible(g_hToolbar);
        PostMessage(hwnd, WM_SIZE, 0, 0);
        break;
    case ID_UNDO:
        Edit_Undo(g_hCanvasWnd);
        break;
    case ID_REDO:
        ASSERT(!"Not implemented yet!");
        break;
    case ID_CUT:
        SendMessage(g_hCanvasWnd, WM_CUT, 0, 0);
        break;
    case ID_COPY:
        SendMessage(g_hCanvasWnd, WM_COPY, 0, 0);
        break;
    case ID_PASTE:
        SendMessage(g_hCanvasWnd, WM_PASTE, 0, 0);
        break;
    case ID_DELETE:
        SendMessage(g_hCanvasWnd, WM_CLEAR, 0, 0);
        break;
    case ID_PRINTPREVIEW:
    case ID_PRINT:
    case ID_PROPERTIES:
    case ID_FIND:
    case ID_REPLACE:
    case ID_HELP:
        ASSERT(!"Not implemented yet!");
        break;
    case ID_SELECTALL:
        SendMessage(g_hCanvasWnd, EM_SETSEL, 0, -1);
        break;
    case ID_PAGESETUP:
        ASSERT(!"Not implemented yet!");
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
        updateCommandUI(hwnd, NULL);
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
        // TODO: Save the position of the main window
        g_profile.nWindowX = rcWnd.left;
        g_profile.nWindowY = rcWnd.top;
    }
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    RECT rc, rcWnd, rcStatus, rcToolbar;

    GetClientRect(hwnd, &rc);
    GetWindowRect(hwnd, &rcWnd);

    if (g_hMainWnd)
    {
        // TODO: Save the main window size
        if (!IsIconic(hwnd) && !IsZoomed(hwnd))
        {
            g_profile.nWindowCX = rcWnd.right - rcWnd.left;
            g_profile.nWindowCY = rcWnd.bottom - rcWnd.top;
        }
        g_profile.bMaximized = IsZoomed(hwnd);
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

        // TODO: Set the status bar parts
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
    destroyControls(hwnd);
    PostQuitMessage(0);
}

void OnInitMenu(HWND hwnd, HMENU hMenu)
{
    INT i, nRecentCount = Recent_GetCount(g_profile.pRecent);
    HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
    INT nFileMenuCount = GetMenuItemCount(hFileMenu);
    HMENU hRecentMenu = GetSubMenu(hFileMenu, nFileMenuCount - 3);
    LPCTSTR pszRecent;
    TCHAR szText[MAX_PATH];

    while (DeleteMenu(hRecentMenu, 0, MF_BYPOSITION))
        ;

    for (i = 0; i < nRecentCount; ++i)
    {
        pszRecent = Recent_GetAt(g_profile.pRecent, i);
        if (!pszRecent)
            continue;

        StringCchPrintf(szText, _countof(szText), TEXT("&%c   %s"),
            TEXT("123456789ABCDEFGHIJK")[i], PathFindFileName(pszRecent));
        AppendMenu(hRecentMenu, MF_STRING | MF_ENABLED, ID_RECENT_01 + i, szText);
    }

    if (Recent_GetCount(g_profile.pRecent) == 0)
    {
        AppendMenu(hRecentMenu, MF_STRING | MF_GRAYED, 0, LoadStringDx(IDS_NONE));
    }

    updateCommandUI(hwnd, hMenu);
}

LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    ASSERT(pnmhdr);

    switch (pnmhdr->code)
    {
    case TTN_NEEDTEXT:
        {
            TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pnmhdr;
            LPTSTR text;

            // TODO: set tooltip text for the command
            pTTT->hinst = g_hInstance;
            text = getCommandText(idFrom, FALSE);
            if (text)
            {
                pTTT->lpszText = text;
            }
        }
    }

    return 0;
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
        HANDLE_MSG(hwnd, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_MOVE, OnMove);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(hwnd, WM_INITMENU, OnInitMenu);
        HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    case WM_MENUSELECT:
        OnMenuSelect(hwnd, wParam, lParam);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL createMainWnd(HINSTANCE hInst, INT nCmdShow)
{
    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    DWORD exstyle = 0;
    INT x = g_profile.nWindowX, y = g_profile.nWindowY;
    INT cx = g_profile.nWindowCX, cy = g_profile.nWindowCY;
    HWND hwnd = CreateWindowEx(exstyle, DX_APP_MAINWND_CLASSNAME, LoadStringDx(IDS_APPNAME), style,
                               x, y, cx, cy, NULL, NULL, hInst, NULL);
    if (!hwnd)
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILCREATEWND), NULL, MB_ICONERROR);
        return FALSE;
    }

    RepositionWindowDx(hwnd);

    if (g_profile.bMaximized)
        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    else
        ShowWindow(hwnd, nCmdShow);

    UpdateWindow(hwnd);
    g_hMainWnd = hwnd;
    return TRUE;
}

BOOL registerClasses(HINSTANCE hInst)
{
    // TODO: Register the window classes that the application uses
    WNDCLASSEX wcx = { sizeof(wcx), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS };
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = hInst;
    //wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcx.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIN));
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    //wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    //wcx.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wcx.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcx.lpszClassName = DX_APP_MAINWND_CLASSNAME;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx))
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILREGCLASS), NULL, MB_ICONERROR);
        return FALSE;
    }

    return registerControls(hInst);
}

BOOL doInitApp(HINSTANCE hInst, LPSTR lpCmdLine, INT nCmdShow)
{
    g_hInstance = hInst;
    InitCommonControls();

    doInitFramework();

    loadProfile(&g_profile, DX_APP_MAX_RECENTS);

    if (!registerClasses(hInst))
        return FALSE;

    if (!createMainWnd(hInst, nCmdShow))
        return FALSE;

    if (!doParseCommandLine())
        return FALSE;

    return TRUE;
}

INT doExitApp(INT ret)
{
    saveProfile(&g_profile);

    if (g_profile.pRecent)
    {
        Recent_Delete(g_profile.pRecent);
        g_profile.pRecent = NULL;
    }

    doFreeFramework();
    return ret;
}

INT doRun(void)
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (g_hMainWnd && TranslateAccelerator(g_hMainWnd, g_hAccel, &msg))
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return doExitApp((INT)msg.wParam);
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    INT ret;

    if (!doInitApp(hInstance, lpCmdLine, nCmdShow))
        return doExitApp(-1);

    ret = doRun();

#if (WINVER >= 0x0500) && !defined(NDEBUG) && 1
    // for detecting object leak (Windows only)
    {
        HANDLE hProcess = GetCurrentProcess();
        TRACEA("Count of GDI objects: %ld\n", GetGuiResources(hProcess, GR_GDIOBJECTS));
        TRACEA("Count of USER objects: %ld\n", GetGuiResources(hProcess, GR_USEROBJECTS));
    }
#endif

#if defined(_MSC_VER) && !defined(NDEBUG) && 1
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}
