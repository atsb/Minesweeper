#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "res.h"
#include "pref.h"
#include "util.h"
#include "grafix.h"
#include "context.h"

extern CHAR szClass[cchNameMax];
extern CHAR szTime[cchNameMax];
extern CHAR szDefaultName[cchNameMax];
extern HINSTANCE hInst;
extern HWND hwndMain;
extern HMENU hMenu;
extern BOOL fEGA;
extern PREF Preferences;
extern CHAR szIniFile[];

INT Rnd(INT rndMax)
{
    if (rndMax <= 0)
        return 0;
    return rand() % rndMax;
}

void ReportErr(WORD idErr)
{
    CHAR szMsg[cchMsgMax];
    CHAR szMsgTitle[cchMsgMax];

    if (idErr < ID_ERR_MAX)
    {
        if (LoadStringA(hInst, idErr, szMsg, ARRAYSIZE(szMsg)) == 0)
            wsprintfA(szMsg, "Error: %u", (unsigned)idErr);
    }
    else
    {
        LoadStringA(hInst, ID_ERR_UNKNOWN, szMsgTitle, ARRAYSIZE(szMsgTitle));
        wsprintfA(szMsg, szMsgTitle, idErr);
    }

    if (LoadStringA(hInst, ID_ERR_TITLE, szMsgTitle, ARRAYSIZE(szMsgTitle)) == 0)
        lstrcpynA(szMsgTitle, "Minesweeper Error", ARRAYSIZE(szMsgTitle));

    MessageBoxA(hwndMain, szMsg, szMsgTitle, MB_OK | MB_ICONERROR);
}

void LoadSz(WORD id, CHAR *sz)
{
    if (LoadStringA(hInst, id, sz, cchMsgMax) == 0)
        ReportErr(1001);
}

void InitConst(void)
{
    srand((unsigned)GetTickCount());

    LoadSz(ID_GAMENAME, szClass);
    LoadSz(ID_MSG_SEC, szTime);
    LoadSz(ID_NAME_DEFAULT, szDefaultName);

    fEGA = GetSystemMetrics(SM_CYSCREEN) < 351;
    dypCaption = GetSystemMetrics(SM_CYCAPTION) + 1;
    dypMenu = GetSystemMetrics(SM_CYMENU) + 1;
    dxpBorder = DpiScale(2);
}

void CheckEm(WORD idm, BOOL fCheck)
{
    if (hMenu != NULL)
        CheckMenuItem(hMenu, idm, MF_BYCOMMAND | (fCheck ? MF_CHECKED : MF_UNCHECKED));
}

void SetMenuBar(INT fActive)
{
    Preferences.fMenu = fActive;
    FixMenus();
    SetMenu(hwndMain, FMenuOn() ? hMenu : NULL);
    AdjustWindow(fResize);
}

static BOOL BuildSiblingPath(const CHAR *extension, CHAR *path, size_t cchPath)
{
    DWORD len;
    CHAR *slash;
    size_t used;

    len = GetModuleFileNameA(hInst, path, (DWORD)cchPath);
    if (len == 0 || len >= cchPath)
        return FALSE;

    slash = strrchr(path, '\\');
    if (slash == NULL)
        slash = strrchr(path, '/');
    if (slash == NULL)
        return FALSE;

    used = (size_t)(slash + 1 - path);
    path[used] = '\0';
    if (used + strlen(extension) + 1 > cchPath)
        return FALSE;
    strcat_s(path, cchPath, extension);
    return TRUE;
}

void DoAbout(void)
{
    CHAR szVersion[cchMsgMax];
    CHAR szCredit[cchMsgMax];
    CHAR szAbout[cchMsgMax * 2 + 4];

    LoadSz(ID_MSG_VERSION, szVersion);
    LoadSz(ID_MSG_CREDIT, szCredit);
    wsprintfA(szAbout, "%s\n\n%s", szVersion, szCredit);
    MessageBoxA(hwndMain, szAbout, szVersion, MB_OK | MB_ICONINFORMATION);
}

void DoHelp(WORD wCommand, LPARAM lParam)
{
    CHAR szHelpFile[MAX_PATH];
    HINSTANCE hResult;

    UNREFERENCED_PARAMETER(lParam);

    if (wCommand == HELP_QUIT)
        return;

    if (!BuildSiblingPath("help\\winmine.rtf", szHelpFile, ARRAYSIZE(szHelpFile)))
    {
        ReportErr(1000);
        return;
    }

    hResult = ShellExecuteA(hwndMain, "open", szHelpFile, NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)hResult <= 32)
    {
        MessageBoxA(hwndMain,
                    "The Minesweeper help document could not be opened.",
                    szClass,
                    MB_OK | MB_ICONWARNING);
    }
}

INT GetDlgInt(HWND hDlg, INT dlgID, INT numLo, INT numHi)
{
    UINT value;
    BOOL translated;

    value = GetDlgItemInt(hDlg, dlgID, &translated, FALSE);
    if (!translated)
        return numLo;
    if (value < (UINT)numLo)
        return numLo;
    if (value > (UINT)numHi)
        return numHi;
    return (INT)value;
}
