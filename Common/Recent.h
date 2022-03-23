#pragma once

struct RECENT;
typedef struct RECENT *PRECENT;

PRECENT Recent_New(INT nCapacity);
INT Recent_GetCapacity(PRECENT pRecent);
INT Recent_GetCount(PRECENT pRecent);
LPCTSTR Recent_GetAt(PRECENT pRecent, INT i);
void Recent_Print(PRECENT pRecent);
INT Recent_Find(PRECENT pRecent, LPCTSTR psz);
void Recent_Add(PRECENT pRecent, LPCTSTR psz);
void Recent_Remove(PRECENT pRecent, LPCTSTR psz);
void Recent_Delete(PRECENT pRecent);
void Recent_UnitTest();
