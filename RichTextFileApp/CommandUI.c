#include "stdafx.h"

extern HWND g_hMainWnd;
extern HWND g_hCanvasWnd;
extern HWND g_hRebar;
extern HWND g_hToolbars[DX_APP_NUM_TOOLBARS];
extern HWND g_hStatusBar;

static HFONT s_hCanvasFont = NULL;

#define IHIML_SMALL 0
#define IHIML_LARGE 1
static HIMAGELIST s_himls[2][DX_APP_NUM_TOOLBARS] = { NULL };
static SIZE s_sizImageSize[2] = {
    { 16, 16 }, // IHIML_SMALL
    { 24, 24 }, // IHIML_LARGE
};
#define rgbMaskColor RGB(255, 0, 255)

#ifdef DX_APP_USE_TEST_CTRLS
    HWND g_hwndTestCtrls[DX_APP_USE_TEST_CTRLS];
#endif

///////////////////////////////////////////////////////////////////////////////
// COMMAND UI

// image list index
#define IML_STD 0
#define IML_HIST 1

// a pair of command id and string id
typedef struct CommandUI
{
    INT id, ids;
#ifndef DX_APP_NEED_DIET
    INT iImageList; // IML_*
    INT iIcon;
    HBITMAP hbmIcon;
#endif
} CommandUI;

// TODO: Add more entries...
// NOTE: The resource string IDS_TOOL_... must be in form of "(command name)|(command description)".
static CommandUI s_CommandUI[] =
{
#ifdef DX_APP_NEED_DIET
#define DEFINE_COMMAND_UI(id, ids, iImageList, iIcon) { id, ids },
#else
#define DEFINE_COMMAND_UI(id, ids, iImageList, iIcon) { id, ids, iImageList, iIcon },
#endif
#include "CommandUI.dat"
#undef DEFINE_COMMAND_UI
};

void dumpCommandUI(void)
{
    TRACEA("---[CommandUI.tsv]---(FROM HERE)---\n");
#ifdef DX_APP_NEED_DIET
    TRACEA("%s\t%s\t%s\t%s\t%s\t%s\n", "(id)", "(id-dec)", "(id-hex)", "(ids)", "(ids-dec)", "(ids-hex)");
#define DEFINE_COMMAND_UI(id, ids, iImageList, iIcon) TRACEA("%s\t%d\t0x%X\t%s\t%d\t0x%X\n", #id, id, id, #ids, ids, ids);
#else
    TRACEA("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "(id)", "(id-dec)", "(id-hex)", "(ids)", "(ids-dec)", "(ids-hex)", "(image-list-index)", "(icon-index-in-image-list)");
#define DEFINE_COMMAND_UI(id, ids, iImageList, iIcon) TRACEA("%s\t%d\t0x%X\t%s\t%d\t0x%X\t%d\t%d\n", #id, id, id, #ids, ids, ids, iImageList, iIcon);
#endif
#include "CommandUI.dat"
#undef DEFINE_COMMAND_UI
    TRACEA("---[CommandUI.tsv]---(DOWN TO HERE)---\n");
}

static CommandUI *findCommand(INT id)
{
    for (size_t i = 0; i < _countof(s_CommandUI); ++i)
    {
        if (id == s_CommandUI[i].id)
        {
            return &s_CommandUI[i];
        }
    }
    return NULL;
}

LPTSTR getCommandText(INT id, BOOL bDetail)
{
    CommandUI *info = findCommand(id);
    if (info)
    {
        LPTSTR text = LoadStringDx(info->ids);
        TCHAR *pch = _tcschr(text, TEXT('|'));
        if (pch)
        {
            *pch = 0;
            if (bDetail)
                return pch + 1;
        }
        return text;
    }
    return NULL;
}

static void enableCommand(INT id, BOOL bEnabled, HMENU hMenu)
{
    ARRAY_FOREACH(HWND hwndTB, g_hToolbars, {
        SendMessage(hwndTB, TB_ENABLEBUTTON, id, bEnabled);
    });

    if (bEnabled)
        EnableMenuItem(hMenu, id, MF_ENABLED);
    else
        EnableMenuItem(hMenu, id, MF_GRAYED);
}

static void checkCommand(INT id, BOOL bChecked, HMENU hMenu)
{
    ARRAY_FOREACH(HWND hwndTB, g_hToolbars, {
        SendMessage(hwndTB, TB_CHECKBUTTON, id, bChecked);
    });

    if (bChecked)
        CheckMenuItem(hMenu, id, MF_CHECKED);
    else
        CheckMenuItem(hMenu, id, MF_UNCHECKED);
}

static void hideCommand(INT id, HMENU hMenu)
{
    ARRAY_FOREACH(HWND hwndTB, g_hToolbars, {
        SendMessage(hwndTB, TB_HIDEBUTTON, id, TRUE);
    });

    DeleteMenu(hMenu, id, MF_BYCOMMAND);
}

void loadMenuBitmaps(HMENU hMenu)
{
#ifndef DX_APP_NEED_DIET
    INT i, id, nCount = GetMenuItemCount(hMenu);
    MENUITEMINFO mii = { 
        sizeof(mii), 
        MIIM_FTYPE | MIIM_BITMAP | MIIM_CHECKMARKS
    };
    CommandUI *info;
    INT cx, cy, cxyPadding = 2;
    HBITMAP hbm;
    HDC hdc;
    HGDIOBJ hbmOld;
    UINT uStyle;

    hdc = CreateCompatibleDC(NULL);
    for (i = 0; i < nCount; ++i)
    {
        if (!GetMenuItemInfo(hMenu, i, TRUE, &mii))
            continue;
        if ((mii.fType & MFT_SEPARATOR))
            continue;

        id = GetMenuItemID(hMenu, i);
        info = findCommand(id);
        if (!info)
            continue;
        if (info->iImageList == -1 || info->iIcon == -1)
            continue;

        //cx = GetSystemMetrics(SM_CXMENUCHECK);
        //cy = GetSystemMetrics(SM_CYMENUCHECK);
        cx = s_sizImageSize[IHIML_SMALL].cx;
        cy = s_sizImageSize[IHIML_SMALL].cy;
        //TRACEA("%d, %d\n", cx, cy);

        hbm = Create24BppBitmapDx(cx + cxyPadding * 2, cy + cxyPadding * 2);
        if (!hbm)
            continue;

        uStyle = ILD_NORMAL;

        hbmOld = SelectObject(hdc, hbm);
        {
            SelectObject(hdc, GetStockBrush(WHITE_BRUSH));
            SelectObject(hdc, GetStockPen(BLACK_PEN));
            Rectangle(hdc, 0, 0, cx + cxyPadding * 2, cy + cxyPadding * 2);
            ImageList_DrawEx(s_himls[IHIML_SMALL][info->iImageList], info->iIcon, 
                hdc, cxyPadding, cxyPadding, cx, cy,
                CLR_NONE, GetSysColor(COLOR_HIGHLIGHT), uStyle);
        }
        SelectObject(hdc, hbmOld);

        info->hbmIcon = hbm;
        mii.hbmpItem = hbm;
        SetMenuItemInfo(hMenu, i, TRUE, &mii);
    }
    DeleteDC(hdc);
#endif
}

void updateCommandUI(HWND hwnd, HMENU hMenu)
{
    if (!hMenu)
        hMenu = GetMenu(g_hMainWnd);

    // TODO: Update UI status
    enableCommand(ID_UNDO, Edit_CanUndo(g_hCanvasWnd), hMenu);
    enableCommand(ID_REDO, (BOOL)SendMessage(g_hCanvasWnd, EM_CANREDO, 0, 0), hMenu);
    enableCommand(ID_PRINTPREVIEW, FALSE, hMenu);
    enableCommand(ID_PRINT, FALSE, hMenu);
    enableCommand(ID_PROPERTIES, FALSE, hMenu);
    enableCommand(ID_FIND, FALSE, hMenu);
    enableCommand(ID_REPLACE, FALSE, hMenu);
    enableCommand(ID_HELP, FALSE, hMenu);
    enableCommand(ID_PAGESETUP, FALSE, hMenu);

    // TODO: Add toolbar commands
    checkCommand(ID_TOOLBAR1, IsWindowVisible(g_hToolbars[0]), hMenu);
    checkCommand(ID_TOOLBAR2, IsWindowVisible(g_hToolbars[1]), hMenu);
#ifdef DX_APP_USE_TEST_CTRLS
    static_assert(DX_APP_NUM_TOOLBARS == 3, "TODO:");
    checkCommand(ID_TOOLBAR3, IsWindowVisible(g_hToolbars[2]), hMenu);
#else
    static_assert(DX_APP_NUM_TOOLBARS == 2, "TODO:");
#endif

    checkCommand(ID_STATUSBAR, IsWindowVisible(g_hStatusBar), hMenu);
    hideCommand(ID_TOOLBAR4, hMenu);

    loadMenuBitmaps(hMenu);
}

void showToolbar(INT index, BOOL bShow)
{
    HWND hwndTB;
    ASSERT(index < DX_APP_NUM_TOOLBARS);
    hwndTB = g_hToolbars[index];
    if (bShow)
    {
        ShowWindow(hwndTB, SW_SHOWNOACTIVATE);
        SendMessage(g_hRebar, RB_SHOWBAND, index, TRUE);
    }
    else
    {
        ShowWindow(hwndTB, SW_HIDE);
        SendMessage(g_hRebar, RB_SHOWBAND, index, FALSE);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CONTROLS

BOOL doCreateImageLists(void)
{
    INT idBitmap;
    HIMAGELIST himl;
    BOOL bLarge;

    bLarge = FALSE;
    idBitmap = IDB_SMALLTOOLBAR1;
    himl = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
        s_sizImageSize[bLarge].cx, 0, rgbMaskColor, IMAGE_BITMAP,
        LR_CREATEDIBSECTION);
    if (!himl)
        return FALSE;
    s_himls[bLarge][0] = himl;

    bLarge = FALSE;
    idBitmap = IDB_SMALLTOOLBAR2;
    himl = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
        s_sizImageSize[bLarge].cx, 0, rgbMaskColor, IMAGE_BITMAP,
        LR_CREATEDIBSECTION);
    if (!himl)
        return FALSE;
    s_himls[bLarge][1] = himl;

    bLarge = TRUE;
    idBitmap = IDB_LARGETOOLBAR1;
    himl = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
        s_sizImageSize[bLarge].cx, 0, rgbMaskColor, IMAGE_BITMAP,
        LR_CREATEDIBSECTION);
    if (!himl)
        return FALSE;
    s_himls[bLarge][0] = himl;

    bLarge = TRUE;
    idBitmap = IDB_LARGETOOLBAR2;
    himl = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
        s_sizImageSize[bLarge].cx, 0, rgbMaskColor, IMAGE_BITMAP,
        LR_CREATEDIBSECTION);
    if (!himl)
        return FALSE;
    s_himls[bLarge][1] = himl;

    return TRUE;
}

HWND doCreateToolbar1(HWND hwnd, INT index, BOOL bHasRebar)
{
    HWND hwndToolbar = NULL;
    DWORD style, exstyle;
    INT id;
    BOOL bStandardButtons = FALSE;  // TODO: Modify
    BOOL bUseLargeButtons = TRUE;   // TODO: Modify
    BOOL bAddString = FALSE;        // TODO: Modify
    BOOL bList = FALSE;             // TODO: Modify
    // TODO: Modify toolbar buttons
    static TBBUTTON buttons[] =
    {
        // { image index, command id, button state, BTNS_, ... }
        { STD_FILENEW, ID_NEW, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_FILEOPEN, ID_OPEN, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_FILESAVE, ID_SAVE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_PRINTPRE, ID_PRINTPREVIEW, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_PRINT, ID_PRINT, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_UNDO, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_REDOW, ID_REDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_CUT, ID_CUT, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_COPY, ID_COPY, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_PASTE, ID_PASTE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_DELETE, ID_DELETE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_PROPERTIES, ID_PROPERTIES, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_FIND, ID_FIND, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { STD_REPLACE, ID_REPLACE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { -1, -1, TBSTATE_ENABLED, BTNS_SEP },
        { STD_HELP, ID_HELP, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
    };
    size_t i, k;
    LPTSTR text;

    // TODO: Invalidate iString's
    for (i = 0; i < _countof(buttons); ++i)
    {
        buttons[i].iString = -1;
    }

    // TODO: Create hwndToolbar
    style = WS_CHILD | CCS_TOP | TBS_HORZ | TBS_TOOLTIPS | CCS_NORESIZE;
    if (bHasRebar)
        style |= CCS_NORESIZE | CCS_NODIVIDER;
    if (bList && bAddString)
        style |= TBSTYLE_LIST;
    exstyle = 0;
    id = IDW_TOOLBAR1;
    hwndToolbar = CreateWindowEx(exstyle, TOOLBARCLASSNAME, NULL,
                                 style, 0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!hwndToolbar)
        return NULL;

    // TODO: Initialize toolbar
    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SetWindowLongPtr(hwndToolbar, GWL_STYLE, GetWindowStyle(hwndToolbar) | TBSTYLE_FLAT);

    if (bAddString)
    {
        for (k = 0; k < _countof(buttons); ++k)
        {
            if (buttons[k].fsStyle & BTNS_SEP)
                continue;

            text = getCommandText(buttons[k].idCommand, FALSE);
            buttons[k].iString = (INT)SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM)text);
        }
    }

    // Enable multiple image lists
    SendMessage(hwndToolbar, CCM_SETVERSION, 5, 0);

    if (bStandardButtons) // standard buttons
    {
        if (bUseLargeButtons)
        {
            TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_STD_LARGE_COLOR };
            //TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_VIEW_LARGE_COLOR };
            //TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_HIST_LARGE_COLOR };
            SendMessage(hwndToolbar, TB_ADDBITMAP, 0, (LPARAM)&AddBitmap);
        }
        else
        {
            TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_STD_SMALL_COLOR };
            //TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_VIEW_SMALL_COLOR };
            //TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_HIST_SMALL_COLOR };
            SendMessage(hwndToolbar, TB_ADDBITMAP, 0, (LPARAM)&AddBitmap);
        }

        SendMessage(hwndToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);
    }
    else // non-standard
    {
        SIZE siz = s_sizImageSize[bUseLargeButtons];
        SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(siz.cx, siz.cy));
        SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)s_himls[bUseLargeButtons][index]);
    }

    SendMessage(hwndToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);

    // TODO: Set extended style
    {
        DWORD extended = 0;
        if (bList)
            extended |= TBSTYLE_EX_MIXEDBUTTONS; // BTNS_SHOWTEXT works
        extended |= TBSTYLE_EX_DRAWDDARROWS; // BTNS_DROPDOWN and BTNS_WHOLEDROPDOWN will work
        //extended |= TBSTYLE_EX_HIDECLIPPEDBUTTONS;
        SendMessage(hwndToolbar, TB_SETEXTENDEDSTYLE, 0, extended);
    }

    return hwndToolbar;
}

HWND doCreateToolbar2(HWND hwnd, INT index, BOOL bHasRebar)
{
    HWND hwndToolbar = NULL;
    DWORD style, exstyle;
    INT id;
    BOOL bUseLargeButtons = TRUE;   // TODO: Modify
    BOOL bAddString = FALSE;        // TODO: Modify
    BOOL bList = FALSE;             // TODO: Modify
    // TODO: Modify toolbar buttons
    static TBBUTTON buttons[] =
    {
        // { image index, command id, button state, BTNS_, ... }
        { HIST_BACK, ID_BACK, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { HIST_FORWARD, ID_FORWARD, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { HIST_FAVORITES, ID_FAVORITE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
    };
    size_t i, k;
    LPTSTR text;

    // TODO: Invalidate iString's
    for (i = 0; i < _countof(buttons); ++i)
    {
        buttons[i].iString = -1;
    }

    // TODO: Create hwndToolbar
    style = WS_CHILD | CCS_TOP | TBS_HORZ | TBS_TOOLTIPS | CCS_NORESIZE;
    if (bHasRebar)
        style |= CCS_NORESIZE | CCS_NODIVIDER;
    if (bList && bAddString)
        style |= TBSTYLE_LIST;
    exstyle = 0;
    id = IDW_TOOLBAR2;
    hwndToolbar = CreateWindowEx(exstyle, TOOLBARCLASSNAME, NULL,
                                 style, 0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!hwndToolbar)
        return NULL;

    // TODO: Initialize toolbar
    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SetWindowLongPtr(hwndToolbar, GWL_STYLE, GetWindowStyle(hwndToolbar) | TBSTYLE_FLAT);

    if (bAddString)
    {
        for (k = 0; k < _countof(buttons); ++k)
        {
            if (buttons[k].fsStyle & BTNS_SEP)
                continue;

            text = getCommandText(buttons[k].idCommand, FALSE);
            buttons[k].iString = (INT)SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM)text);
        }
    }

    // Enable multiple image lists
    SendMessage(hwndToolbar, CCM_SETVERSION, 5, 0);

    {
        SIZE siz = s_sizImageSize[bUseLargeButtons];
        SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(siz.cx, siz.cy));
        SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)s_himls[bUseLargeButtons][index]);
    }

    SendMessage(hwndToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);

    // TODO: Set extended style
    {
        DWORD extended = 0;
        if (bList)
            extended |= TBSTYLE_EX_MIXEDBUTTONS; // BTNS_SHOWTEXT works
        extended |= TBSTYLE_EX_DRAWDDARROWS; // BTNS_DROPDOWN and BTNS_WHOLEDROPDOWN will work
        //extended |= TBSTYLE_EX_HIDECLIPPEDBUTTONS;
        SendMessage(hwndToolbar, TB_SETEXTENDEDSTYLE, 0, extended);
    }

    return hwndToolbar;
}

#ifdef DX_APP_USE_TEST_CTRLS
HWND doCreateToolbar3(HWND hwnd, INT index, BOOL bHasRebar)
{
    HWND hwndToolbar = NULL;
    DWORD style, exstyle;
    INT id;
    BOOL bUseLargeButtons = TRUE;   // TODO: Modify
    BOOL bAddString = FALSE;        // TODO: Modify
    BOOL bList = FALSE;             // TODO: Modify
    // TODO: Modify toolbar buttons
    static TBBUTTON buttons[DX_APP_USE_TEST_CTRLS + 1] =
    {
        // { image index, command id, button state, BTNS_, ... }
        { -1, ID_TEST_9, 0, BTNS_SEP },
        { -1, ID_TEST_1, 0, BTNS_SEP },
        { -1, ID_TEST_2, 0, BTNS_SEP },
        { -1, ID_TEST_3, 0, BTNS_SEP },
    };
    size_t i, k;
    LPTSTR text;
    SIZE siz;
    INT cxPadding = 4;

    // TODO: Invalidate iString's
    for (i = 0; i < _countof(buttons); ++i)
    {
        buttons[i].iString = -1;
    }

    // TODO: Create hwndToolbar
    style = WS_CHILD | CCS_TOP | TBS_HORZ | TBS_TOOLTIPS | CCS_NORESIZE;
    if (bHasRebar)
        style |= CCS_NORESIZE | CCS_NODIVIDER;
    if (bList && bAddString)
        style |= TBSTYLE_LIST;
    exstyle = 0;
    id = IDW_TOOLBAR2;
    hwndToolbar = CreateWindowEx(exstyle, TOOLBARCLASSNAME, NULL,
                                 style, 0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!hwndToolbar)
        return NULL;

    // TODO: Initialize toolbar
    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SetWindowLongPtr(hwndToolbar, GWL_STYLE, GetWindowStyle(hwndToolbar) | TBSTYLE_FLAT);

    for (i = 0; i < DX_APP_USE_TEST_CTRLS; ++i)
    {
        TCHAR szText[MAX_PATH];
        HFONT hFont = GetStockFont(DEFAULT_GUI_FONT);

        if (i == 0)
        {
            style = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL;
            exstyle = WS_EX_CLIENTEDGE;
            g_hwndTestCtrls[i] = CreateWindowEx(exstyle, TEXT("EDIT"), NULL, style,
                0, 0, 0, 0, hwndToolbar, (HMENU)(INT_PTR)(IDW_TEST_CTRL_1 + i), g_hInstance, NULL);
            SetWindowFont(g_hwndTestCtrls[i], hFont, TRUE);
            buttons[i].iBitmap = 80 + cxPadding; // control width
            continue;
        }

        StringCchPrintf(szText, _countof(szText), TEXT("Test %d"), (INT)i);
        siz = GetTextExtentDx(szText, hFont);
        siz.cx += 8;
        siz.cy += 8;
        buttons[i].iBitmap = siz.cx + cxPadding; // control width

        style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
        exstyle = 0;
        g_hwndTestCtrls[i] = CreateWindowEx(exstyle, TEXT("BUTTON"), szText, style,
            0, 0, 0, 0, hwndToolbar, (HMENU)(INT_PTR)(buttons[i].idCommand), g_hInstance, NULL);
        if (g_hwndTestCtrls[i] == NULL)
            return FALSE;

        SetWindowFont(g_hwndTestCtrls[i], hFont, TRUE);
    }

    if (bAddString)
    {
        for (k = 0; k < _countof(buttons); ++k)
        {
            if (buttons[k].fsStyle & BTNS_SEP)
                continue;

            text = getCommandText(buttons[k].idCommand, FALSE);
            buttons[k].iString = (INT)SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM)text);
        }
    }

    // Enable multiple image lists
    SendMessage(hwndToolbar, CCM_SETVERSION, 5, 0);

    SendMessage(hwndToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);

    for (i = 0; i < DX_APP_USE_TEST_CTRLS; ++i)
    {
        RECT rc;
        SendMessage(hwndToolbar, TB_GETITEMRECT, (INT)i, (LPARAM)&rc);
        rc.right -= cxPadding;

        MoveWindow(g_hwndTestCtrls[i], rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }

    // TODO: Set extended style
    {
        DWORD extended = 0;
        if (bList)
            extended |= TBSTYLE_EX_MIXEDBUTTONS; // BTNS_SHOWTEXT works
        extended |= TBSTYLE_EX_DRAWDDARROWS; // BTNS_DROPDOWN and BTNS_WHOLEDROPDOWN will work
        //extended |= TBSTYLE_EX_HIDECLIPPEDBUTTONS;
        SendMessage(hwndToolbar, TB_SETEXTENDEDSTYLE, 0, extended);
    }

    return hwndToolbar;
}
#endif // def DX_APP_USE_TEST_CTRLS

BOOL doCreateRebar(HWND hwnd)
{
    DWORD style, exstyle;

    if (!doCreateImageLists())
        return FALSE;

    // TODO: Create a Rebar control
    style = WS_CHILD | WS_VISIBLE | RBS_BANDBORDERS | CCS_NODIVIDER | RBS_AUTOSIZE;
    exstyle = WS_EX_TOOLWINDOW;
    g_hRebar = CreateWindowEx(exstyle, REBARCLASSNAME, NULL, style,
        0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)IDW_REBAR, g_hInstance, NULL);
    if (!g_hRebar)
        return FALSE;

    {
        REBARINFO info = { sizeof(info) };
        SendMessage(g_hRebar, RB_SETBARINFO, 0, (LPARAM)&info);
    }

    // TODO: Create toolbars
    g_hToolbars[0] = doCreateToolbar1(g_hRebar, 0, g_hRebar != NULL);
    if (!g_hToolbars[0])
        return FALSE;

    g_hToolbars[1] = doCreateToolbar2(g_hRebar, 1, g_hRebar != NULL);
    if (!g_hToolbars[1])
        return FALSE;

#ifdef DX_APP_USE_TEST_CTRLS
    static_assert(DX_APP_NUM_TOOLBARS == 3, "TODO:");
    g_hToolbars[2] = doCreateToolbar3(g_hRebar, 2, g_hRebar != NULL);
    if (!g_hToolbars[1])
        return FALSE;
#else
    static_assert(DX_APP_NUM_TOOLBARS == 2, "TODO:");
#endif

    {
        SIZE siz;
        INT index = 0;
        ARRAY_FOREACH(HWND hwndTB, g_hToolbars, {
            REBARBANDINFO band = { sizeof(band) };
            SendMessage(hwndTB, TB_GETMAXSIZE, 0, (LPARAM)&siz);
            band.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
            band.hwndChild = hwndTB;
            band.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;
            band.cxMinChild = siz.cx;
            band.cyMinChild = siz.cy;
            band.cx = siz.cx;
            SendMessage(g_hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&band);
            ++index;
        });
    }

    return TRUE;
}

BOOL registerControls(HINSTANCE hInst)
{
    // TODO:
    LoadLibraryA("RICHED32");
    return TRUE;
}

BOOL createControls(HWND hwnd)
{
    DWORD style, exstyle;
    INT id;

    // TODO: Create canvas font
    {
        LOGFONT lf = { 24 };
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
        s_hCanvasFont = CreateFontIndirect(&lf);
        if (!s_hCanvasFont)
            return FALSE;
    }

    // TODO: Create canvas window
    style = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL;
    exstyle = WS_EX_CLIENTEDGE;
    id = IDW_CANVAS;
    g_hCanvasWnd = CreateWindowEx(exstyle, TEXT("RichEdit"), NULL, style, 0, 0, 0, 0,
        hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!g_hCanvasWnd)
        return FALSE;

    // We have to receive EN_CHANGE from richedit
    {
        DWORD dwMask = (DWORD)SendMessage(g_hCanvasWnd, EM_GETEVENTMASK, 0, 0);
        SendMessage(g_hCanvasWnd, EM_SETEVENTMASK, 0,
                    dwMask | ENM_CHANGE | ENM_DROPFILES | ENM_MOUSEEVENTS);
    }

    // TODO: Set canvas font
    //SetWindowFont(g_hCanvasWnd, GetStockFont(DEFAULT_GUI_FONT), TRUE);
    SetWindowFont(g_hCanvasWnd, s_hCanvasFont, TRUE);

    if (!doCreateRebar(hwnd))
        return FALSE;

    // TODO: Create the status bar
    style = WS_CHILD | SBS_SIZEGRIP;
    exstyle = 0;
    id = IDW_STATUSBAR;
    g_hStatusBar = CreateStatusWindow(style, NULL, hwnd, id);
    if (!g_hStatusBar)
        return FALSE;

    return TRUE;
}

void destroyControls(HWND hwnd)
{
    DestroyWindow(g_hCanvasWnd);

    ARRAY_FOREACH(HWND hwndTB, g_hToolbars, {
        DestroyWindow(hwndTB);
    });

#ifdef DX_APP_USE_TEST_CTRLS
    ARRAY_FOREACH(HWND hwndTest, g_hwndTestCtrls, {
        DestroyWindow(hwndTest);
    });
#endif

#ifndef DX_APP_NEED_DIET
    ARRAY_FOREACH(CommandUI ui, s_CommandUI, {
        if (ui.hbmIcon)
            DeleteObject(ui.hbmIcon);
    });
#endif

    DestroyWindow(g_hRebar);
    DestroyWindow(g_hStatusBar);

    {
        size_t i;
        for (i = 0; i < 2; ++i)
        {
            ARRAY_FOREACH(HIMAGELIST himl, s_himls[i], {
                ImageList_Destroy(himl);
            });
        }
    }

    if (s_hCanvasFont)
    {
        DeleteObject(s_hCanvasFont);
        s_hCanvasFont = NULL;
    }
}

// WM_MENUSELECT
void OnMenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UINT uItem = LOWORD(wParam), fuFlags = HIWORD(wParam);
    HMENU hmenu = (HMENU)lParam;
    LPTSTR text;
    UINT dummy[2] = { 0 };

    if (fuFlags & MF_POPUP)
        uItem = GetMenuItemID(hmenu, uItem);

    if (fuFlags & MF_SYSMENU)
    {
        SendMessage(g_hStatusBar, SB_SETTEXT, 255 | SBT_NOBORDERS, (LPARAM)TEXT(""));
        SendMessage(g_hStatusBar, SB_SIMPLE, TRUE, 0);
        MenuHelp(WM_MENUSELECT, wParam, lParam, NULL, g_hInstance, g_hStatusBar, dummy);
        return;
    }

    if (fuFlags == 0xFFFF && !hmenu)
    {
        SendMessage(g_hStatusBar, SB_SIMPLE, FALSE, 0);
        PostMessage(hwnd, WM_COMMAND, 0, 0);
        PostMessage(hwnd, WM_SIZE, 0, 0);
        return;
    }

    text = getCommandText(uItem, TRUE);
    if (text)
    {
        SendMessage(g_hStatusBar, SB_SETTEXT, 255 | SBT_NOBORDERS, (LPARAM)text);
        SendMessage(g_hStatusBar, SB_SIMPLE, TRUE, 0);
        return;
    }

    SendMessage(g_hStatusBar, SB_SETTEXT, 255 | SBT_NOBORDERS, (LPARAM)TEXT(""));
    SendMessage(g_hStatusBar, SB_SIMPLE, TRUE, 0);
}
