#include "stdafx.h"

#define CLASS_NAME TEXT("Canvas")

extern HINSTANCE g_hInstance;
extern HWND g_hCanvasWnd;
extern INT g_iActivePanel;

static BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    return TRUE;
}

static void OnDrawClient(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    {
        TCHAR sz[64];
        StringCchPrintf(sz, _countof(sz), TEXT("(Panel %d Canvas)"), (g_iActivePanel + 1));
        DrawText(hdc, sz, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    MoveToEx(hdc, rc.left, rc.top, NULL);
    LineTo(hdc, rc.right, rc.bottom);
    MoveToEx(hdc, rc.right, rc.top, NULL);
    LineTo(hdc, rc.left, rc.bottom);
}

static void OnPaint(HWND hwnd)
{
    HDC hdc;
    PAINTSTRUCT ps;

    hdc = BeginPaint(hwnd, &ps);
    if (hdc)
    {
        OnDrawClient(hwnd, hdc);
        EndPaint(hwnd, &ps);
    }
}

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

HWND createCanvasWnd(HWND hwnd)
{
    HWND hCanvasWnd;
    DWORD style, exstyle;

    style = WS_CHILD | WS_VISIBLE;
    exstyle = WS_EX_CLIENTEDGE;
    hCanvasWnd = CreateWindowEx(exstyle, CLASS_NAME, NULL, style, 0, 0, 0, 0,
        hwnd, NULL, g_hInstance, NULL);
    return hCanvasWnd;
}

BOOL registerCanvas(HINSTANCE hInst)
{
    WNDCLASSEX wcx = { sizeof(wcx), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS };
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = g_hInstance;
    //wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    //wcx.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    //wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcx.hbrBackground = (HBRUSH)(COLOR_3DDKSHADOW + 1);
    //wcx.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcx.lpszClassName = CLASS_NAME;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx))
    {
        MessageBox(NULL, LoadStringDx(IDS_FAILREGCLASS), NULL, MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}
