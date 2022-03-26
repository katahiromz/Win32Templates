#include "stdafx.h"

extern HWND g_hMainWnd;
extern HWND g_hCanvasWnd;
extern HWND g_hToolbar;
extern HWND g_hStatusBar;

static HFONT s_hCanvasFont = NULL;

static HIMAGELIST s_himlToolbar = NULL; // image list for toolbar

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
    SendMessage(g_hToolbar, TB_ENABLEBUTTON, id, bEnabled);

    if (bEnabled)
        EnableMenuItem(hMenu, id, MF_ENABLED);
    else
        EnableMenuItem(hMenu, id, MF_GRAYED);
}

static void checkCommand(INT id, BOOL bChecked, HMENU hMenu)
{
    SendMessage(g_hToolbar, TB_CHECKBUTTON, id, bChecked);

    if (bChecked)
        CheckMenuItem(hMenu, id, MF_CHECKED);
    else
        CheckMenuItem(hMenu, id, MF_UNCHECKED);
}

static void hideCommand(INT id, HMENU hMenu)
{
    SendMessage(g_hToolbar, TB_HIDEBUTTON, id, TRUE);
    DeleteMenu(hMenu, id, MF_BYCOMMAND);
}

void updateCommandUI(HWND hwnd, HMENU hMenu)
{
    if (!hMenu)
        hMenu = GetMenu(g_hMainWnd);

    // TODO: Update UI status
    enableCommand(ID_UNDO, Edit_CanUndo(g_hCanvasWnd), hMenu);
    enableCommand(ID_REDO, FALSE, hMenu);
    enableCommand(ID_PRINTPREVIEW, FALSE, hMenu);
    enableCommand(ID_PRINT, FALSE, hMenu);
    enableCommand(ID_PROPERTIES, FALSE, hMenu);
    enableCommand(ID_FIND, FALSE, hMenu);
    enableCommand(ID_REPLACE, FALSE, hMenu);
    enableCommand(ID_HELP, FALSE, hMenu);
    enableCommand(ID_PAGESETUP, FALSE, hMenu);
    checkCommand(ID_TOOLBAR, IsWindowVisible(g_hToolbar), hMenu);
    checkCommand(ID_STATUSBAR, IsWindowVisible(g_hStatusBar), hMenu);
    hideCommand(ID_REDO, hMenu);
}

///////////////////////////////////////////////////////////////////////////////
// CONTROLS

BOOL doCreateToolbar(HWND hwnd)
{
    DWORD style, exstyle;
    INT id;
    BOOL bStandardButtons = TRUE;  // TODO: Modify
    BOOL bUseLargeButtons = FALSE;   // TODO: Modify
    BOOL bAddString = FALSE;        // TODO: Modify
    BOOL bList = FALSE;             // TODO: Modify
    // TODO: Modify toolbar buttons
    static TBBUTTON buttons[] =
    {
        // { image index, command id, button state, BTNS_, ... }
#if 0
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
#else
        { VIEW_LARGEICONS, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_SMALLICONS, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_LIST, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_DETAILS, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_SORTNAME, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_SORTSIZE, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_SORTDATE, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_SORTTYPE, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_PARENTFOLDER, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_NETCONNECT, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_NETDISCONNECT, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
        { VIEW_NEWFOLDER, ID_UNDO, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE },
#endif
    };
    size_t i, k;
    LPTSTR text;

    // TODO: Invalidate iString's
    for (i = 0; i < _countof(buttons); ++i)
    {
        buttons[i].iString = -1;
    }

    // TODO: Create g_hToolbar
    style = WS_CHILD | CCS_TOP | TBS_HORZ | TBS_TOOLTIPS;
    if (bList && bAddString)
        style |= TBSTYLE_LIST;
    exstyle = 0;
    id = IDW_TOOLBAR;
    g_hToolbar = CreateWindowEx(exstyle, TOOLBARCLASSNAME, NULL,
                                style, 0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!g_hToolbar)
        return FALSE;

    // TODO: Initialize toolbar
    SendMessage(g_hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SetWindowLongPtr(g_hToolbar, GWL_STYLE, GetWindowStyle(g_hToolbar) | TBSTYLE_FLAT);

    if (bAddString)
    {
        for (k = 0; k < _countof(buttons); ++k)
        {
            if (buttons[k].fsStyle & BTNS_SEP)
                continue;

            text = getCommandText(buttons[k].idCommand, FALSE);
            buttons[k].iString = (INT)SendMessage(g_hToolbar, TB_ADDSTRING, 0, (LPARAM)text);
        }
    }

    // Enable multiple image lists
    SendMessage(g_hToolbar, CCM_SETVERSION, 5, 0);

    if (bStandardButtons) // standard buttons
    {
        if (bUseLargeButtons)
        {
            //TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_STD_LARGE_COLOR };
            TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_VIEW_LARGE_COLOR };
            SendMessage(g_hToolbar, TB_ADDBITMAP, 3, (LPARAM)&AddBitmap);
        }
        else
        {
            //TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_STD_SMALL_COLOR };
            TBADDBITMAP AddBitmap = { HINST_COMMCTRL, IDB_VIEW_SMALL_COLOR };
            SendMessage(g_hToolbar, TB_ADDBITMAP, 3, (LPARAM)&AddBitmap);
        }

        SendMessage(g_hToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);
    }
    else // non-standard
    {
        INT idBitmap, nButtonImageWidth, nButtonImageHeight;
        // TODO: Change
        //COLORREF rgbMaskColor = RGB(255, 0, 255);
        COLORREF rgbMaskColor = CLR_NONE;

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

        SendMessage(g_hToolbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(nButtonImageWidth, nButtonImageHeight));

        s_himlToolbar = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(idBitmap),
                                            nButtonImageWidth, 0, rgbMaskColor, IMAGE_BITMAP, 
                                            LR_CREATEDIBSECTION);
        if (!s_himlToolbar)
            return FALSE;

        SendMessage(g_hToolbar, TB_SETIMAGELIST, 0, (LPARAM)s_himlToolbar);
        SendMessage(g_hToolbar, TB_ADDBUTTONS, _countof(buttons), (LPARAM)&buttons);
    }

    // TODO: Set extended style
    {
        DWORD extended = 0;
        if (bList)
            extended |= TBSTYLE_EX_MIXEDBUTTONS; // BTNS_SHOWTEXT works
        extended |= TBSTYLE_EX_DRAWDDARROWS; // BTNS_DROPDOWN and BTNS_WHOLEDROPDOWN will work
        //extended |= TBSTYLE_EX_HIDECLIPPEDBUTTONS;
        SendMessage(g_hToolbar, TB_SETEXTENDEDSTYLE, 0, extended);
    }

    return TRUE;
}

BOOL registerControls(HINSTANCE hInst)
{
    // TODO:
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
    g_hCanvasWnd = CreateWindowEx(exstyle, TEXT("EDIT"), NULL, style, 0, 0, 0, 0,
        hwnd, (HMENU)(INT_PTR)id, g_hInstance, NULL);
    if (!g_hCanvasWnd)
        return FALSE;

    // TODO: Set canvas font
    //SetWindowFont(g_hCanvasWnd, GetStockFont(DEFAULT_GUI_FONT), TRUE);
    SetWindowFont(g_hCanvasWnd, s_hCanvasFont, TRUE);

    if (!doCreateToolbar(hwnd))
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
    DestroyWindow(g_hToolbar);
    DestroyWindow(g_hStatusBar);

    if (s_himlToolbar)
    {
        ImageList_Destroy(s_himlToolbar);
        s_himlToolbar = NULL;
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
