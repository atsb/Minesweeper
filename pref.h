#ifndef WINMINE_PREF_H
#define WINMINE_PREF_H

#define cchNameMax 32

typedef struct
{
    INT  wGameType;
    INT Mines;
    INT Height;
    INT Width;
    INT xWindow;
    INT yWindow;
    INT fSound;
    BOOL fMark;
    BOOL fTick;
    BOOL fMenu;
    BOOL fColor;
    INT rgTime[3];
    CHAR szBegin[cchNameMax];
    CHAR szInter[cchNameMax];
    CHAR szExpert[cchNameMax];
} PREF;

void ReadPreferences(void);
void WritePreferences(void);
BOOL FWindowPositionSaved(void);

#endif
