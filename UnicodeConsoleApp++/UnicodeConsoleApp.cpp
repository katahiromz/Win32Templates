#include "stdafx.h"

void version(void)
{
    // TODO: Show version info
#ifdef _WIN32
    std::wcout << LoadStringDx(IDS_VERSION) << std::endl;
#else
    std::wcout << L"UnicodeConsoleApp.cpp Ver.0.0" << std::endl;
#endif
}

void help(void)
{
    // TODO: Show usage
    std::cout <<
        "Usage: ConsoleApp [Options] \"your_file.txt\"\n"
        "Options:\n"
        "--help    Show this message.\n"
        "--version Show version info.\n"
        "\n"
        "Contact: katayama.hirofumi.mz@gmail.com" << std::endl;
}

int do_it(const std::wstring& filename, int flags)
{
    // TODO: Do something
    return 0;
}

int wmain(int argc, wchar_t **argv)
{
    int ret, flags = 0;
    std::wstring filename;

    if (argc <= 1)
    {
        help();
        return 0;
    }

    // TODO: Parse command line
    for (int iarg = 1; iarg < argc; ++iarg)
    {
        std::wstring arg = argv[iarg];
        if (arg == L"--help" || arg == L"/?")
        {
            help();
            return 0;
        }
        if (arg == L"--version")
        {
            version();
            return 0;
        }
        if (arg[0] == L'-')
        {
            std::wcout << L"ConsoleApp: Invalid argument '" << arg << L"'" << std::endl;
            return -1;
        }
        filename = arg;
    }

    ret = do_it(filename, flags);

#if (WINVER >= 0x0500) && !defined(NDEBUG) && 1
    // for detecting object leak (Windows only)
    {
        HANDLE hProcess = GetCurrentProcess();
        DebugPrintfA("Count of GDI objects: %ld\n", GetGuiResources(hProcess, GR_GDIOBJECTS));
        DebugPrintfA("Count of USER objects: %ld\n", GetGuiResources(hProcess, GR_USEROBJECTS));
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
