#include <windows.h>
#include <string.h>
#include "main.h"
#include "res.h"
#include "rtns.h"
#include "grafix.h"
#include "pref.h"
#include "sound.h"

BOOL fUpdateIni = FALSE;
CHAR szIniFile[MAX_PATH];

extern CHAR szDefaultName[];
extern CHAR szClass[];
extern INT xBoxMac;
extern INT yBoxMac;
extern PREF Preferences;
extern BOOL fEGA;

#define iszPrefGame 0
#define iszPrefMines 1
#define iszPrefHeight 2
#define iszPrefWidth 3
#define iszPrefxWindow 4
#define iszPrefyWindow 5
#define iszPrefSound 6
#define iszPrefMark 7
#define iszPrefMenu 8
#define iszPrefTick 9
#define iszPrefColor 10
#define iszPrefBeginTime 11
#define iszPrefBeginName 12
#define iszPrefInterTime 13
#define iszPrefInterName 14
#define iszPrefExpertTime 15
#define iszPrefExpertName 16
#define iszPrefMax 17

static BOOL fWindowPositionSaved = FALSE;

static const CHAR *const rgszPref[iszPrefMax] =
{
    "Difficulty",
    "Mines",
    "Height",
    "Width",
    "Xpos",
    "Ypos",
    "Sound",
    "Mark",
    "Menu",
    "Tick",
    "Color",
    "Time1",
    "Name1",
    "Time2",
    "Name2",
    "Time3",
    "Name3"
};

static void InitIniPath(void)
{
    DWORD cch;
    CHAR *slash;

    if (szIniFile[0] != '\0')
        return;

    cch = GetModuleFileNameA(NULL, szIniFile, ARRAYSIZE(szIniFile));
    if (cch == 0 || cch >= ARRAYSIZE(szIniFile))
    {
        lstrcpynA(szIniFile, "entpack.ini", ARRAYSIZE(szIniFile));
        return;
    }

    slash = strrchr(szIniFile, '\\');
    if (slash == NULL)
        slash = strrchr(szIniFile, '/');

    if (slash != NULL)
        lstrcpynA(slash + 1, "entpack.ini", (int)(ARRAYSIZE(szIniFile) - (slash + 1 - szIniFile)));
    else
        lstrcpynA(szIniFile, "entpack.ini", ARRAYSIZE(szIniFile));
}

static INT ReadInt(INT iszPref, INT valDefault, INT valMin, INT valMax)
{
    INT value;

    value = (INT)GetPrivateProfileIntA(szClass, rgszPref[iszPref], valDefault, szIniFile);
    if (value < valMin)
        value = valMin;
    else if (value > valMax)
        value = valMax;
    return value;
}

static INT ReadBool(INT iszPref, INT valDefault)
{
    return ReadInt(iszPref, valDefault, 0, 1);
}

static void ReadSz(INT iszPref, CHAR *szRet)
{
    GetPrivateProfileStringA(szClass, rgszPref[iszPref], szDefaultName,
                             szRet, cchNameMax, szIniFile);
    szRet[cchNameMax - 1] = '\0';
}

static BOOL FPreferenceExists(INT iszPref)
{
    CHAR szValue[2];

    return GetPrivateProfileStringA(szClass, rgszPref[iszPref], "",
                                   szValue, ARRAYSIZE(szValue), szIniFile) != 0;
}

BOOL FWindowPositionSaved(void)
{
    return fWindowPositionSaved;
}

void ReadPreferences(void)
{
    InitIniPath();

#ifdef JAPAN
    yBoxMac = Preferences.Height = ReadInt(iszPrefHeight, 8, 8, fEGA ? 16 : 22);
#else
    yBoxMac = Preferences.Height = ReadInt(iszPrefHeight, 8, 8, fEGA ? 16 : 25);
#endif

    xBoxMac = Preferences.Width = ReadInt(iszPrefWidth, 8, 8, 30);
    Preferences.wGameType = ReadInt(iszPrefGame, wGameBegin, wGameBegin, wGameOther);
    {
        INT maxMines = (Preferences.Height * Preferences.Width) - 1;
        if (maxMines < 10)
            maxMines = 10;
        Preferences.Mines = ReadInt(iszPrefMines, 10, 10, maxMines);
    }
    fWindowPositionSaved = FPreferenceExists(iszPrefxWindow) &&
                           FPreferenceExists(iszPrefyWindow);

    Preferences.xWindow = ReadInt(iszPrefxWindow, 80, 0, 1024);
    Preferences.yWindow = ReadInt(iszPrefyWindow, 80, 0, 1024);
    Preferences.fSound = ReadInt(iszPrefSound, 0, 0, fsoundOn);
    Preferences.fMark = ReadBool(iszPrefMark, TRUE);
    Preferences.fTick = ReadBool(iszPrefTick, FALSE);
    Preferences.fMenu = ReadInt(iszPrefMenu, fmenuAlwaysOn, fmenuAlwaysOn, fmenuOn);

    Preferences.rgTime[wGameBegin] = ReadInt(iszPrefBeginTime, 999, 0, 999);
    Preferences.rgTime[wGameInter] = ReadInt(iszPrefInterTime, 999, 0, 999);
    Preferences.rgTime[wGameExpert] = ReadInt(iszPrefExpertTime, 999, 0, 999);

    ReadSz(iszPrefBeginName, Preferences.szBegin);
    ReadSz(iszPrefInterName, Preferences.szInter);
    ReadSz(iszPrefExpertName, Preferences.szExpert);

    {
        HDC hDC = GetDC(NULL);
        Preferences.fColor = ReadBool(iszPrefColor, GetDeviceCaps(hDC, NUMCOLORS) != 2);
        ReleaseDC(NULL, hDC);
    }

    if (FSoundOn())
        Preferences.fSound = FInitTunes();
}

static void WriteInt(INT iszPref, INT val)
{
    CHAR szVal[16];
    wsprintfA(szVal, "%d", val);
    WritePrivateProfileStringA(szClass, rgszPref[iszPref], szVal, szIniFile);
}

static void WriteSz(INT iszPref, const CHAR *sz)
{
    WritePrivateProfileStringA(szClass, rgszPref[iszPref], sz, szIniFile);
}

void WritePreferences(void)
{
    InitIniPath();

    WriteInt(iszPrefGame, Preferences.wGameType);
    WriteInt(iszPrefHeight, Preferences.Height);
    WriteInt(iszPrefWidth, Preferences.Width);
    WriteInt(iszPrefMines, Preferences.Mines);
    WriteInt(iszPrefMark, Preferences.fMark);
    WriteInt(iszPrefMenu, Preferences.fMenu);
    WriteInt(iszPrefTick, Preferences.fTick);
    WriteInt(iszPrefColor, Preferences.fColor);
    WriteInt(iszPrefxWindow, Preferences.xWindow);
    WriteInt(iszPrefyWindow, Preferences.yWindow);

    WriteInt(iszPrefBeginTime, Preferences.rgTime[wGameBegin]);
    WriteInt(iszPrefInterTime, Preferences.rgTime[wGameInter]);
    WriteInt(iszPrefExpertTime, Preferences.rgTime[wGameExpert]);

    WriteSz(iszPrefBeginName, Preferences.szBegin);
    WriteSz(iszPrefInterName, Preferences.szInter);
    WriteSz(iszPrefExpertName, Preferences.szExpert);

    if (FSoundSwitchable())
        WriteInt(iszPrefSound, Preferences.fSound);
}
