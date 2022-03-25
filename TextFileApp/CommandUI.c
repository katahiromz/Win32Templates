#include "stdafx.h"

extern HWND g_hMainWnd;
extern HWND g_hCanvasWnd;
extern HWND g_hToolbar;
extern HWND g_hStatusBar;

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

void updateCommandUI(HWND hwnd, HMENU hMenu)
{
    if (!hMenu)
        hMenu = GetMenu(g_hMainWnd);

    // TODO: Update UI status by using enableCommand and checkCommand
    checkCommand(ID_TOOLBAR, IsWindowVisible(g_hToolbar), hMenu);
    checkCommand(ID_STATUSBAR, IsWindowVisible(g_hStatusBar), hMenu);
    enableCommand(ID_UNDO, Edit_CanUndo(g_hCanvasWnd), hMenu);
    enableCommand(ID_REDO, FALSE, hMenu);
    enableCommand(ID_PRINTPREVIEW, FALSE, hMenu);
    enableCommand(ID_PRINT, FALSE, hMenu);
    enableCommand(ID_PROPERTIES, FALSE, hMenu);
    enableCommand(ID_FIND, FALSE, hMenu);
    enableCommand(ID_REPLACE, FALSE, hMenu);
    enableCommand(ID_HELP, FALSE, hMenu);
    enableCommand(ID_PAGESETUP, FALSE, hMenu);
}
