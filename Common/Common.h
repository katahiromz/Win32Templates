#pragma once

#include "Config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Base64.h"

#ifdef WIN32
    #include <windows.h>
    #include <windowsx.h>
    #include <commctrl.h>
    #include <commdlg.h>
    #include <shellapi.h>
    #include <shlwapi.h>
    #include <crtdbg.h>
    #include <tchar.h>
    #include <strsafe.h>
    #include "Bitmap.h"
    #include "Recent.h"
    #include "Shortcut.h"
    #include "Utils.h"
#else
    #define TEXT(text) text
#endif
