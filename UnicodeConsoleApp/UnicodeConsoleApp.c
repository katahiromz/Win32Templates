#include "stdafx.h"

void version(void)
{
    // TODO: Show version info
#ifdef _WIN32
    _putts(LoadStringDx(IDS_VERSION));
#else
    puts("UnicodeConsoleApp ver.0.0");
#endif
}

void help(void)
{
    // TODO: Show usage
    printf(
        "Usage: UnicodeConsoleApp [Options] \"your_file.txt\"\n"
        "Options:\n"
        "--help    Show this message.\n"
        "--version Show version info.\n"
        "\n"
        "Contact: katayama.hirofumi.mz@gmail.com\n");
}

int do_it(const wchar_t *filename, int flags)
{
    // TODO: Do something
    return 0;
}

int wmain(int argc, wchar_t **argv)
{
    int ret, iarg, flags = 0;
    const wchar_t *filename = NULL;

    if (argc <= 1)
    {
        help();
        return 0;
    }

    // TODO: Parse command line
    for (iarg = 1; iarg < argc; ++iarg)
    {
        const wchar_t *arg = argv[iarg];
        if (wcscmp(arg, L"--help") == 0 || wcscmp(arg, L"/?") == 0)
        {
            help();
            return 0;
        }
        if (wcscmp(arg, L"--version") == 0)
        {
            version();
            return 0;
        }
        if (arg[0] == L'-')
        {
            wprintf(L"ConsoleApp: Invalid argument '%s'\n", arg);
            return -1;
        }
        filename = arg;
    }

    ret = do_it(filename, flags);

#if (WINVER >= 0x0500) && !defined(NDEBUG) && 1
    // for detecting object leak (Windows only)
    {
        HANDLE hProcess = GetCurrentProcess();
        TRACEA("Count of GDI objects: %ld\n", GetGuiResources(hProcess, GR_GDIOBJECTS));
        TRACEA("Count of USER objects: %ld\n", GetGuiResources(hProcess, GR_USEROBJECTS));
    }
#endif

#if defined(_MSC_VER) && !defined(NDEBUG) && 1
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}

int main(int argc, char **argv)
{
    INT wargc;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    int ret = wmain(wargc, wargv);
    LocalFree(wargv);
    return ret;
}
