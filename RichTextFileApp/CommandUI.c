#include "stdafx.h"

extern HWND g_hMainWnd;
extern HWND g_hCanvasWnd;
extern HWND g_hRebar;
extern HWND g_hToolbars[DX_APP_NUM_TOOLBARS];
extern HWND g_hStatusBar;

static HFONT s_hCanvasFont = NULL;

static HIMAGELIST s_himlToolbar = NULL;
static HIMAGELIST s_himlToolbar2 = NULL;

///////////////////////////////////////////////////////////////////////////////
// COMMAND UI

// a pair of command id and string id
typedef struct CommandUI
{
    INT id, ids;
} CommandUI;

// TODO: Add more entries...
// NOTE: The resource string IDS_TOOL_... must be in form of "(command name)|(command description)".
static CommandUI s_CommandUI[] =
{
#define DEFINE_COMMAND_UI(id, ids) { id, ids },
#include "CommandUI.dat"
#undef DEFINE_COMMAND_UI
};

void dumpCommandUI(void)
{
    TRACEA("---[CommandUI.tsv]---(FROM HERE)---\n");
    TRACEA("%s\t%s\t%s\t%s\t%s\t%s\n", "(id)", "(id-dec)", "(id-hex)", "(ids)", "(ids-dec)", "(ids-hex)");
#define DEFINE_COMMAND_UI(id, ids) TRACEA("%s\t%d\t0x%X\t%s\t%d\t0x%X\n", #id, id, id, #ids, ids, ids);
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
    {
        size_t i;
        for (i = 0; i < _countof(g_hToolbars); ++i)
            SendMessage(g_hToolbars[i], TB_ENABLEBUTTON, id, bEnabled);
    }

    if (bEnabled)
        EnableMenuItem(hMenu, id, MF_ENABLED);
    else
        EnableMenuItem(hMenu, id, MF_GRAYED);
}

static void checkCommand(INT id, BOOL bChecked, HMENU hMenu)
{
    {
        size_t i;
        for (i = 0; i < _countof(g_hToolbars); ++i)
            SendMessage(g_hToolbars[i], TB_CHECKBUTTON, id, bChecked);
    }

    if (bChecked)
        CheckMenuItem(hMenu, id, MF_CHECKED);
    else
        CheckMenuItem(hMenu, id, MF_UNCHECKED);
}

static void hideCommand(INT id, HMENU hMenu)
{
    {
        size_t i;
        for (i = 0; i < _countof(g_hToolbars); ++i)
            SendMessage(g_hToolbars[i], TB_HIDEBUTTON, id, TRUE);
    }

    DeleteMenu(hMenu, id, MF_BYCOMMAND);
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
    checkCommand(ID_TOOLBAR, IsWindowVisible(g_hToolbars[0]), hMenu);
    checkCommand(ID_STATUSBAR, IsWindowVisible(g_hStatusBar), hMenu);
    //hideCommand(ID_REDO, hMenu);
}

///////////////////////////////////////////////////////////////////////////////
// CONTROLS

HWND doCreateToolbar(HWND hwnd)
{
    HWND hwndToolbar = NULL;
    DWORD style, exstyle;
    INT id;
    BOOL bStandardButtons = FALSE;  // TODO: Modify
    BOOL bUseLargeButtons = TRUE;   // TODO: Modify
    BOOL bAddString = FALSE;        // TODO: Modify
    BOOL bHasRebar = TRUE;          // TODO: Modify
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
    id = IDW_TOOLBAR;
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
        INT idBitmap, nButtonImageWidth, nButtonImageHeight;
        // TODO: Change
        COLORREF rgbMaskColor = RGB(255, 0, 255);
        //COLORREF rgbMaskColor = CLR_NONE;

        if (bUseLargeButtons)
        {
            idBitmap = IDB_LARGETOOLBAR;
            nButtonImageWidth = nButtonImageHeight = 24;
        }
        else
        {
            idBitmap = IDB_SMALLTOOLBAR;
            nButtonImageWidth = nButtonImageHeight = 16;
        }

        SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(nButtonImageWidth, nButtonImageHeight));

        s_himlToolbar = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
                                            nButtonImageWidth, 0, rgbMaskColor, IMAGE_BITMAP, 
                                            LR_CREATEDIBSECTION);
        if (!s_himlToolbar)
            return NULL;

        SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)s_himlToolbar);
        SendMessage(hwndToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);
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

HWND doCreateToolbar2(HWND hwnd)
{
    HWND hwndToolbar = NULL;
    DWORD style, exstyle;
    INT id;
    BOOL bUseLargeButtons = TRUE;   // TODO: Modify
    BOOL bAddString = FALSE;        // TODO: Modify
    BOOL bHasRebar = TRUE;          // TODO: Modify
    BOOL bList = FALSE;             // TODO: Modify
    // TODO: Modify toolbar buttons
    static TBBUTTON buttons[] =
    {
        // { image index, command id, button state, BTNS_, ... }
        { HIST_BACK, -1, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { HIST_FORWARD, -1, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { HIST_FAVORITES, -1, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { HIST_VIEWTREE, -1, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
    };
    size_t i, k;
    LPTSTR text;

    // TODO: Invalidate iString's
    for (i = 0; i < _countof(buttons); ++i)
    {
        buttons[i].iString = -1;
    }

    // TODO: Create hwndToolbar
    style = WS_CHILD | CCS_TOP | TBS_HORZ | TBS_TOOLTIPS;
    if (bHasRebar)
        style |= CCS_NORESIZE | CCS_NODIVIDER;
    if (bList && bAddString)
        style |= TBSTYLE_LIST;
    exstyle = 0;
    id = IDW_TOOLBAR;
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
        INT idBitmap, nButtonImageWidth, nButtonImageHeight;
        // TODO: Change
        COLORREF rgbMaskColor = RGB(255, 0, 255);
        //COLORREF rgbMaskColor = CLR_NONE;

        if (bUseLargeButtons)
        {
            idBitmap = IDB_LARGETOOLBAR2;
            nButtonImageWidth = nButtonImageHeight = 24;
        }
        else
        {
            idBitmap = IDB_SMALLTOOLBAR2;
            nButtonImageWidth = nButtonImageHeight = 16;
        }

        SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(nButtonImageWidth, nButtonImageHeight));

        s_himlToolbar2 = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
                                             nButtonImageWidth, 0, rgbMaskColor, IMAGE_BITMAP, 
                                             LR_CREATEDIBSECTION);
        if (!s_himlToolbar2)
            return NULL;

        SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)s_himlToolbar2);
        SendMessage(hwndToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);
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

BOOL doCreateRebar(HWND hwnd)
{
    DWORD style = WS_CHILD | WS_VISIBLE | RBS_BANDBORDERS | CCS_NODIVIDER | RBS_AUTOSIZE;
    DWORD exstyle = WS_EX_TOOLWINDOW;
    g_hRebar = CreateWindowEx(exstyle, REBARCLASSNAME, NULL, style,
        0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)IDW_REBAR, g_hInstance, NULL);
    if (!g_hRebar)
        return FALSE;

    {
        REBARINFO info = { sizeof(info) };
        SendMessage(g_hRebar, RB_SETBARINFO, 0, (LPARAM)&info);
    }

    g_hToolbars[0] = doCreateToolbar(g_hRebar);
    if (!g_hToolbars[0])
        return FALSE;

    g_hToolbars[1] = doCreateToolbar2(g_hRebar);
    if (!g_hToolbars[1])
        return FALSE;

    {
        size_t i;
        for (i = 0; i < _countof(g_hToolbars); ++i)
        {
            SIZE siz;
            REBARBANDINFO band = { sizeof(band) };
            HWND hwndTB = g_hToolbars[i];
            SendMessage(hwndTB, TB_GETMAXSIZE, 0, (LPARAM)&siz);
            band.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
            band.hwndChild = hwndTB;
            band.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;
            band.cxMinChild = siz.cx;
            band.cyMinChild = siz.cy;
            band.cx = siz.cx;
            SendMessage(g_hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&band);
        }
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
    SendMessage(g_hCanvasWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_DROPFILES);

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

    {
        size_t i;
        for (i = 0; i < _countof(g_hToolbars); ++i)
            DestroyWindow(g_hToolbars[i]);
    }

    DestroyWindow(g_hRebar);
    DestroyWindow(g_hStatusBar);

    if (s_himlToolbar)
    {
        ImageList_Destroy(s_himlToolbar);
        s_himlToolbar = NULL;
    }
    if (s_himlToolbar2)
    {
        ImageList_Destroy(s_himlToolbar2);
        s_himlToolbar2 = NULL;
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
