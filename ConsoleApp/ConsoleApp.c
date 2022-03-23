#include "stdafx.h"

void version(void)
{
    // TODO: Show version info
#ifdef _WIN32
    _putts(LoadStringDx(IDS_VERSION));
#else
    puts("ConsoleApp Ver.0.0");
#endif
}

void help(void)
{
    // TODO: Show usage
    puts("Usage: ConsoleApp [Options] \"your_file.txt\"\n"
         "Options:\n"
         "--help    Show this message.\n"
         "--version Show version info.\n"
         "\n"
         "Contact: katayama.hirofumi.mz@gmail.com");
}

int do_it(const char *filename, int flags)
{
    // TODO: Do something
    printf("filename: %s\n", filename);
    printf("flags: 0x%X\n", flags);
    return 0;
}

int main(int argc, char **argv)
{
    int ret, iarg, flags = 0;
    const char *filename = NULL;

    if (argc <= 1)
    {
        help();
        return 0;
    }

    // TODO: Parse command line
    for (iarg = 1; iarg < argc; ++iarg)
    {
        const char *arg = argv[iarg];
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "/?") == 0)
        {
            help();
            return 0;
        }
        if (strcmp(arg, "--version") == 0)
        {
            version();
            return 0;
        }
        if (arg[0] == '-')
        {
            printf("ConsoleApp: Invalid argument '%s'\n", arg);
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
