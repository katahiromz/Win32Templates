#include "stdafx.h"

#define PANEL_INDEX 0

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    panel_OnActivated(hwnd, PANEL_INDEX);
    return TRUE;
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case psh1:
        panel_OnBack(hwnd, PANEL_INDEX);
        break;
    case psh2:
        panel_OnForward(hwnd, PANEL_INDEX);
        break;
    }
}

INT_PTR CALLBACK
panel1DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
    default:
        break;
    }
    return 0;
}
