#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <tchar.h>
#include "Recent.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct RECENT
{
    INT nCapacity;
    INT nCount;
    LPTSTR ppsz[ANYSIZE_ARRAY];
} RECENT;

PRECENT Recent_New(INT nCapacity)
{
    PRECENT pRecent = calloc(1, sizeof(RECENT) + (nCapacity - 1) * sizeof(LPTSTR));
    if (!pRecent)
        return NULL;
    pRecent->nCapacity = nCapacity;
    return pRecent;
}

INT Recent_GetCapacity(PRECENT pRecent)
{
    assert(pRecent);
    if (!pRecent)
        return 0;
    return pRecent->nCapacity;
}

INT Recent_GetCount(PRECENT pRecent)
{
    assert(pRecent);
    if (!pRecent)
        return 0;
    return pRecent->nCount;
}

LPCTSTR Recent_GetAt(PRECENT pRecent, INT i)
{
    if (!pRecent || i >= pRecent->nCapacity)
        return NULL;
    return pRecent->ppsz[i];
}

void Recent_Print(PRECENT pRecent)
{
    INT i;
    printf("---: %d\n", pRecent->nCount);
    for (i = 0; i < pRecent->nCapacity; ++i)
    {
        printf("[%d]: %ls\n", i, pRecent->ppsz[i]);
    }
}

INT Recent_Find(PRECENT pRecent, LPCTSTR psz)
{
    INT i;
    for (i = 0; i < pRecent->nCapacity; ++i)
    {
        LPTSTR pszRecent = pRecent->ppsz[i];
        if (!pszRecent)
            continue;
        if (_tcsicmp(pszRecent, psz) == 0)
            return i;
    }
    return -1;
}

#define SWAP(psz1, psz2) do { \
    pszTemp = psz1; \
    psz1 = psz2; \
    psz2 = pszTemp; \
} while (0);

void Recent_Remove(PRECENT pRecent, LPCTSTR psz)
{
    LPTSTR pszTemp;
    INT i, iFound = Recent_Find(pRecent, psz);

    if (iFound < 0)
        return;

    for (i = iFound; i < pRecent->nCapacity - 1; ++i)
    {
        SWAP(pRecent->ppsz[i], pRecent->ppsz[i + 1]);
    }

    pRecent->nCount -= 1;
}

void Recent_Add(PRECENT pRecent, LPCTSTR psz)
{
    LPTSTR pszTemp;
    INT i;

    Recent_Remove(pRecent, psz);

    for (i = pRecent->nCapacity - 1; i > 0; --i)
    {
        SWAP(pRecent->ppsz[i], pRecent->ppsz[i - 1]);
    }

    free(pRecent->ppsz[0]);
    pRecent->ppsz[0] = _tcsdup(psz);

    if (pRecent->nCount < pRecent->nCapacity)
        pRecent->nCount += 1;
}

void Recent_Delete(PRECENT pRecent)
{
    INT i;

    for (i = 0; i < pRecent->nCapacity; ++i)
    {
        free(pRecent->ppsz[i]);
        pRecent->ppsz[i] = NULL;
    }
    free(pRecent);
}

void Recent_UnitTest()
{
    PRECENT pRecent = Recent_New(3);
    assert(pRecent);
    assert(pRecent->nCapacity == 3);
    assert(pRecent->nCount == 0);
    assert(pRecent->ppsz[0] == NULL);
    assert(pRecent->ppsz[1] == NULL);
    assert(pRecent->ppsz[2] == NULL);
    Recent_Add(pRecent, TEXT("A"));
    assert(_tcscmp(pRecent->ppsz[0], TEXT("A")) == 0);
    assert(pRecent->ppsz[1] == NULL);
    assert(pRecent->ppsz[2] == NULL);
    Recent_Add(pRecent, TEXT("B"));
    assert(_tcscmp(pRecent->ppsz[0], TEXT("B")) == 0);
    assert(_tcscmp(pRecent->ppsz[1], TEXT("A")) == 0);
    assert(pRecent->ppsz[2] == NULL);
    Recent_Add(pRecent, TEXT("C"));
    assert(_tcscmp(pRecent->ppsz[0], TEXT("C")) == 0);
    assert(_tcscmp(pRecent->ppsz[1], TEXT("B")) == 0);
    assert(_tcscmp(pRecent->ppsz[2], TEXT("A")) == 0);
    Recent_Remove(pRecent, TEXT("B"));
    assert(_tcscmp(pRecent->ppsz[0], TEXT("C")) == 0);
    assert(_tcscmp(pRecent->ppsz[1], TEXT("A")) == 0);
    assert(_tcscmp(pRecent->ppsz[2], TEXT("B")) == 0);
    Recent_Remove(pRecent, TEXT("C"));
    assert(_tcscmp(pRecent->ppsz[0], TEXT("A")) == 0);
    assert(_tcscmp(pRecent->ppsz[1], TEXT("B")) == 0);
    assert(_tcscmp(pRecent->ppsz[2], TEXT("C")) == 0);
    Recent_Remove(pRecent, TEXT("A"));
    Recent_Delete(pRecent);
    puts("OK");
}

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef UNITTEST
int main(void)
{
    Recent_UnitTest();
    return 0;
}
#endif
