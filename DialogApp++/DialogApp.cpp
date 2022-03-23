#include "stdafx.h"

HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
HICON g_hMainIcon = NULL;

BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hMainWnd = hwnd;
    CenterWindowDx(hwnd);
    DragAcceptFiles(hwnd, TRUE);
    g_hMainIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_hMainIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hMainIcon);
    return TRUE;
}

BOOL OnOK(HWND hwnd)
{
    // TODO:
    return TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        if (OnOK(hwnd))
        {
            EndDialog(hwnd, id);
        }
        break;
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    case psh1:
        MsgBoxDx(hwnd, TEXT("Did it!"), LoadStringDx(IDS_APPNAME), MB_ICONINFORMATION);
        break;
    }
}

void OnDropFiles(HWND hwnd, HDROP hdrop)
{
    TCHAR szFile[MAX_PATH];
    DragQueryFile(hdrop, 0, szFile, _countof(szFile));
    // TODO: szFile
    DragFinish(hdrop);
}

void OnDestroy(HWND hwnd)
{
    if (g_hMainIcon)
    {
        DestroyIcon(g_hMainIcon);
        g_hMainIcon = NULL;
    }
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DROPFILES, OnDropFiles);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    g_hInstance = hInstance;
    InitCommonControls();
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);

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

    return 0;
}
