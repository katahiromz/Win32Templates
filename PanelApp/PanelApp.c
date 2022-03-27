#include "stdafx.h"

#define DX_APP_NUM_PANELS 3

HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HWND g_hCanvasWnd = NULL;
HWND g_hwndPanels[DX_APP_NUM_PANELS] = { NULL };
HICON g_hIcons[3] = { NULL, NULL, NULL };

INT g_iActivePanel = 0;
#define HWND_ACTIVE_PANEL g_hwndPanels[g_iActivePanel]

// Canvas.c
extern BOOL registerCanvas(HINSTANCE hInst);
extern HWND createCanvasWnd(HWND hwnd);

SIZE g_sizInitialDlg = { 0, 0 };

// Panel1.c
extern INT_PTR CALLBACK
panel1DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Panel2.c
extern INT_PTR CALLBACK
panel2DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Panel3.c
extern INT_PTR CALLBACK
panel3DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

DLGPROC panelDlgProcs[DX_APP_NUM_PANELS] =
{
    panel1DialogProc,
    panel2DialogProc,
    panel3DialogProc,
};

void activatePage(INT iPanel)
{
    // TODO: Switch the panel
    ShowWindow(g_hwndPanels[g_iActivePanel], SW_HIDE);
    ShowWindow(g_hwndPanels[iPanel], SW_SHOW);
    g_iActivePanel = iPanel;

    // TODO: Set button icons
    switch (iPanel)
    {
    case 0:
    case 1:
        SendDlgItemMessage(HWND_ACTIVE_PANEL, psh1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIcons[0]);
        SendDlgItemMessage(HWND_ACTIVE_PANEL, psh2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIcons[1]);
        SendDlgItemMessage(HWND_ACTIVE_PANEL, psh8, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIcons[2]);
        break;
    case 2:
        SendDlgItemMessage(HWND_ACTIVE_PANEL, psh1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIcons[0]);
        SendDlgItemMessage(HWND_ACTIVE_PANEL, psh8, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIcons[2]);
        break;
    }

    InvalidateRect(g_hCanvasWnd, NULL, TRUE);
    PostMessage(g_hMainWnd, WM_SIZE, 0, 0);
}

void panel_OnInit(HWND hwndDlg, INT iPanel)
{
    // TODO:
}

BOOL createControls(HWND hwnd)
{
    size_t i;
    HWND hwndDlg;
    g_hCanvasWnd = createCanvasWnd(hwnd);
    if (!g_hCanvasWnd)
        return FALSE;

    for (i = 0; i < DX_APP_NUM_PANELS; ++i)
    {
        hwndDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PANEL1 + i), hwnd, panelDlgProcs[i]);
        if (!hwndDlg)
            return FALSE;
        ShowWindow(hwndDlg, SW_HIDE);
        g_hwndPanels[i] = hwndDlg;
    }
    {
        RECT rc;
        SIZE siz;
        GetWindowRect(HWND_ACTIVE_PANEL, &rc);
        siz.cx = rc.right - rc.left;
        siz.cy = rc.bottom - rc.top;
        g_sizInitialDlg = siz;
    }
    activatePage(0);
    return TRUE;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    g_hMainWnd = hwnd;
    CenterWindowDx(hwnd);

    g_hIcons[0] = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_BACK));
    ASSERT(g_hIcons[0]);
    g_hIcons[1] = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NEXT));
    ASSERT(g_hIcons[1]);
    g_hIcons[2] = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_HAMBURGER));
    ASSERT(g_hIcons[2]);

    if (!createControls(hwnd))
        return FALSE;
    return TRUE;
}

void OnDropFiles(HWND hwnd, HDROP hdrop)
{
    TCHAR szFile[MAX_PATH];
    DragQueryFile(hdrop, 0, szFile, _countof(szFile));
    // TODO: szFile
    DragFinish(hdrop);
}

BOOL About_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    CenterWindowDx(hwnd);
    return TRUE;
}

void About_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
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
AboutDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, About_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, About_OnCommand);
    }
    return 0;
}

void OnAbout(HWND hwnd)
{
    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDialogProc);
}

void OnHamburgerMenu(HWND hwnd)
{
    RECT rc;
    HWND hPsh8 = GetDlgItem(HWND_ACTIVE_PANEL, psh8);
    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUPMENUS));
    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    INT id;
    TPMPARAMS params = { sizeof(params) };
    POINT pt;
    UINT uFlags;

    GetWindowRect(hPsh8, &rc);

    SetForegroundWindow(hwnd);
    pt.x = rc.left;
    pt.y = (rc.top + rc.bottom) / 2;
    uFlags = TPM_VERTICAL | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD;
    params.rcExclude = rc;
    id = TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, &params);
    if (id)
    {
        PostMessage(hwnd, WM_COMMAND, id, 0);
    }

    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case ID_EXIT:
        DestroyWindow(hwnd);
        break;
    case ID_ABOUT:
        OnAbout(hwnd);
        break;
    case ID_TEST:
        MsgBoxDx(hwnd, TEXT("This is a test"), LoadStringDx(IDS_APPNAME), MB_ICONINFORMATION);
        break;
    case psh1: // Back
        switch (g_iActivePanel)
        {
        case 0:
            ASSERT(0);
            break;
        case 1:
            activatePage(0);
            break;
        case 2:
            activatePage(1);
            break;
        }
        break;
    case psh2: // Next or Finish
        switch (g_iActivePanel)
        {
        case 0:
            activatePage(1);
            break;
        case 1:
            activatePage(2);
            break;
        case 2:
            DestroyWindow(g_hMainWnd);
            break;
        }
        break;
    case psh8:
        OnHamburgerMenu(hwnd);
        break;
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

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    RECT rc;
    SIZE siz;
    POINT pt;
    GetClientRect(hwnd, &rc);
    pt.x = rc.left;
    pt.y = rc.top;
    siz.cx = rc.right - rc.left;
    siz.cy = rc.bottom - rc.top;
    MoveWindow(HWND_ACTIVE_PANEL, pt.x, pt.y, g_sizInitialDlg.cx, siz.cy, TRUE);
    MoveWindow(g_hCanvasWnd, pt.x + g_sizInitialDlg.cx, pt.y, siz.cx - g_sizInitialDlg.cx, siz.cy, TRUE);
}

void OnDestroy(HWND hwnd)
{
    DestroyIcon(g_hIcons[0]);
    DestroyIcon(g_hIcons[1]);
    DestroyIcon(g_hIcons[2]);
    PostQuitMessage(0);
}

void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    lpMinMaxInfo->ptMinTrackSize.x = g_sizInitialDlg.cx + 128;
    lpMinMaxInfo->ptMinTrackSize.y = g_sizInitialDlg.cy + 64;
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_DROPFILES, OnDropFiles);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL doCreateMainWnd(HINSTANCE hInstance, INT nCmdShow)
{
    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    DWORD exstyle = 0;
    INT x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    INT cx = 600, cy = 400;
    HWND hwnd = CreateWindowEx(exstyle, MAINWND_CLASSNAME, LoadStringDx(IDS_TITLE), style,
                               x, y, cx, cy, NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILCREATEWND), NULL, MB_ICONERROR);
        return FALSE;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return TRUE;
}

BOOL registerClasses(HINSTANCE hInstance)
{
    WNDCLASSEX wcx = { sizeof(wcx), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS };
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = hInstance;
    //wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    //wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcx.hbrBackground = (HBRUSH)(COLOR_3DDKSHADOW + 1);
    //wcx.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcx.lpszClassName = MAINWND_CLASSNAME;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx))
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILREGCLASS), NULL, MB_ICONERROR);
        return FALSE;
    }

    return registerCanvas(hInstance);
}

BOOL doInitApp(HINSTANCE hInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    if (!registerClasses(hInstance))
        return FALSE;
    if (!doCreateMainWnd(hInstance, nCmdShow))
        return FALSE;
    return TRUE;
}

INT doRun(void)
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
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
    if (!doInitApp(hInstance, lpCmdLine, nCmdShow))
        return -1;
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
