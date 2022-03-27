#pragma once

#include "targetver.h"
#include "Common.h"
#include "resource.h"

#define MAINWND_CLASSNAME TEXT("WindowApp by katahiromz")

#define IDW_STATUSBAR 1

void activatePage(INT iPanel);
void panel_OnActivated(HWND hwndDlg, INT iPanel);
void panel_OnBack(HWND hwndDlg, INT iPanel);
void panel_OnForward(HWND hwndDlg, INT iPanel);
