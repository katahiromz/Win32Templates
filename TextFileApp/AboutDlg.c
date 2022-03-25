#include "stdafx.h"

extern HINSTANCE g_hInstance;

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
about_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, about_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, about_OnCommand);
    }
    return 0;
}

void doAboutDlg(HWND hwnd)
{
    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, about_DialogProc);
}
