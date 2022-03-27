#pragma once

#include "targetver.h"
#include "Common.h"
#include "resource.h"

#define MAINWND_CLASSNAME TEXT("WindowApp by katahiromz")

#define IDW_STATUSBAR 1

void activatePage(INT iPanel);
void panel_OnInit(HWND hwndDlg, INT iPanel);
