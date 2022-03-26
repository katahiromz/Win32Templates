#pragma once

#include "targetver.h"
#include "Common.h"
#include <richedit.h>
#include "resource.h"

// TODO: Modify if necessary
#define DX_APP_MAX_RECENTS              20
#define DX_APP_COMPANY_NAME_IN_ENGLISH  TEXT("Katayama Hirofumi MZ")
#define DX_APP_NAME_IN_ENGLISH          TEXT("RichTextFileApp")
#define DX_APP_MAINWND_CLASSNAME        TEXT("RichTextFileApp by katahiromz")
#define DX_APP_USE_REGISTRY             TRUE
#define DX_APP_USE_TEST_CTRLS           4
//#define DX_APP_NEED_SPEED
//#define DX_APP_NEED_DIET

// The window IDs
#define IDW_CANVAS edt1
#define IDW_TOOLBAR1 rct1
#define IDW_TOOLBAR2 rct2
#define IDW_TOOLBAR3 rct3
#define IDW_TOOLBAR4 rct4
#define IDW_STATUSBAR stc1
#define IDW_REBAR grp1
#define IDW_TEST_CTRL_1 ID_TEST_1
#define IDW_TEST_CTRL_2 ID_TEST_2
#define IDW_TEST_CTRL_3 ID_TEST_3
#define IDW_TEST_CTRL_4 ID_TEST_4

#ifdef DX_APP_USE_TEST_CTRLS
    #define DX_APP_NUM_TOOLBARS 3
#else
    #define DX_APP_NUM_TOOLBARS 2
#endif
