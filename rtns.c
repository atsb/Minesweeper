#include <windows.h>
#include "res.h"
#include "main.h"
#include "rtns.h"
#include "util.h"
#include "grafix.h"
#include "sound.h"
#include "pref.h"

PREF Preferences;
INT xBoxMac;
INT yBoxMac;
INT dxWindow;
INT dyWindow;
INT iButtonCur = iButtonHappy;
INT cBombStart;
INT cBombLeft;
INT cBoxVisit;
INT cBoxVisitMac;
INT cSec;
BOOL fTimer = FALSE;
INT xCur = -1;
INT yCur = -1;
BYTE rgBlk[cBlkMax];

#define iStepMax 100
static INT rgStepX[iStepMax];
static INT rgStepY[iStepMax];
static INT iStepMac;

extern BOOL fBlock;
extern INT fStatus;

#define fCheckWin() (cBoxVisit == cBoxVisitMac)

static void ChangeBlk(INT x, INT y, INT iBlk)
{
    SetBlk(x, y, iBlk);
    DisplayBlk(x, y);
}

void ClearField(void)
{
    INT i;

    for (i = cBlkMax; i-- != 0;)
        rgBlk[i] = (BYTE)iBlkBlankUp;

    for (i = xBoxMac + 2; i-- != 0;)
    {
        SetBorder(i, 0);
        SetBorder(i, yBoxMac + 1);
    }

    for (i = yBoxMac + 2; i-- != 0;)
    {
        SetBorder(0, i);
        SetBorder(xBoxMac + 1, i);
    }
}

static INT CountBombs(INT xCenter, INT yCenter)
{
    INT x;
    INT y;
    INT cBombs = 0;

    for (y = yCenter - 1; y <= yCenter + 1; ++y)
        for (x = xCenter - 1; x <= xCenter + 1; ++x)
            if (fISBOMB(x, y))
                ++cBombs;

    return cBombs;
}

static void ShowBombs(INT iBlk)
{
    INT x;
    INT y;

    for (y = 1; y <= yBoxMac; ++y)
    {
        for (x = 1; x <= xBoxMac; ++x)
        {
            if (!fVISIT(x, y))
            {
                if (fISBOMB(x, y))
                {
                    if (!fGUESSBOMB(x, y))
                        SetBlk(x, y, iBlk);
                }
                else if (fGUESSBOMB(x, y))
                {
                    SetBlk(x, y, iBlkWrong);
                }
            }
        }
    }

    DisplayGrid();
}

static void GameOver(BOOL fWinLose)
{
    fTimer = FALSE;
    DisplayButton(iButtonCur = fWinLose ? iButtonWin : iButtonLose);
    PlayTune(fWinLose ? TUNE_WINGAME : TUNE_LOSEGAME);
    ShowBombs(fWinLose ? iBlkBombUp : iBlkBombDn);

    if (fWinLose && cBombLeft != 0)
        UpdateBombCount(-cBombLeft);

    SetStatusDemo;

    if (fWinLose && Preferences.wGameType != wGameOther &&
        cSec < Preferences.rgTime[Preferences.wGameType])
    {
        Preferences.rgTime[Preferences.wGameType] = cSec;
        DoEnterName();
        DoDisplayBest();
    }
}

void DoTimer(void)
{
    if (fTimer && cSec < 999)
    {
        ++cSec;
        DisplayTime();
        PlayTune(TUNE_TICK);
    }
}

static void StepXY(INT x, INT y)
{
    INT cBombs;
    INT iBlk = (y << 5) + x;
    BLK blk = rgBlk[iBlk];

    if ((blk & MaskVisit) != 0 ||
        ((blk & MaskData) == iBlkMax) ||
        ((blk & MaskData) == iBlkBombUp))
        return;

    ++cBoxVisit;
    rgBlk[iBlk] = (BYTE)(MaskVisit | (cBombs = CountBombs(x, y)));
    DisplayBlk(x, y);

    if (cBombs != 0)
        return;

    rgStepX[iStepMac] = x;
    rgStepY[iStepMac] = y;
    if (++iStepMac == iStepMax)
        iStepMac = 0;
}

static void StepBox(INT x, INT y)
{
    INT iStepCur = 0;

    iStepMac = 1;
    StepXY(x, y);

    if (++iStepCur != iStepMac)
    {
        while (iStepCur != iStepMac)
        {
            x = rgStepX[iStepCur];
            y = rgStepY[iStepCur];

            StepXY(x - 1, --y);
            StepXY(x, y);
            StepXY(x + 1, y);
            StepXY(x - 1, ++y);
            StepXY(x + 1, y);
            StepXY(x - 1, ++y);
            StepXY(x, y);
            StepXY(x + 1, y);

            if (++iStepCur == iStepMax)
                iStepCur = 0;
        }
    }
}

static void StepSquare(INT x, INT y)
{
    if (fISBOMB(x, y))
    {
        if (cBoxVisit == 0)
        {
            INT xT;
            INT yT;
            for (yT = 1; yT < yBoxMac; ++yT)
                for (xT = 1; xT < xBoxMac; ++xT)
                    if (!fISBOMB(xT, yT))
                    {
                        IBLK(x, y) = (BYTE)iBlkBlankUp;
                        SetBomb(xT, yT);
                        StepBox(x, y);
                        return;
                    }
        }
        else
        {
            ChangeBlk(x, y, MaskVisit | iBlkExplode);
            GameOver(fLose);
        }
    }
    else
    {
        StepBox(x, y);
        if (fCheckWin())
            GameOver(fWin);
    }
}

static INT CountMarks(INT xCenter, INT yCenter)
{
    INT x;
    INT y;
    INT cBombs = 0;

    for (y = yCenter - 1; y <= yCenter + 1; ++y)
        for (x = xCenter - 1; x <= xCenter + 1; ++x)
            if (fGUESSBOMB(x, y))
                ++cBombs;

    return cBombs;
}

static void StepBlock(INT xCenter, INT yCenter)
{
    INT x;
    INT y;
    BOOL fGameOver = FALSE;

    if (!fVISIT(xCenter, yCenter) ||
        iBLK(xCenter, yCenter) != CountMarks(xCenter, yCenter))
    {
        TrackMouse(-2, -2);
        return;
    }

    for (y = yCenter - 1; y <= yCenter + 1; ++y)
    {
        for (x = xCenter - 1; x <= xCenter + 1; ++x)
        {
            if (!fGUESSBOMB(x, y) && fISBOMB(x, y))
            {
                fGameOver = TRUE;
                ChangeBlk(x, y, MaskVisit | iBlkExplode);
            }
            else
            {
                StepBox(x, y);
            }
        }
    }

    if (fGameOver)
        GameOver(fLose);
    else if (fCheckWin())
        GameOver(fWin);
}

void StartGame(void)
{
    BOOL fAdjust;
    INT x;
    INT y;

    fTimer = FALSE;
    fAdjust = (Preferences.Width != xBoxMac || Preferences.Height != yBoxMac)
                  ? (fResize | fDisplay)
                  : fDisplay;

    xBoxMac = Preferences.Width;
    yBoxMac = Preferences.Height;

    ClearField();
    iButtonCur = iButtonHappy;
    cBombStart = Preferences.Mines;

    do
    {
        do
        {
            x = Rnd(xBoxMac) + 1;
            y = Rnd(yBoxMac) + 1;
        }
        while (fISBOMB(x, y));

        SetBomb(x, y);
    }
    while (--cBombStart);

    cSec = 0;
    cBombLeft = cBombStart = Preferences.Mines;
    cBoxVisit = 0;
    cBoxVisitMac = (xBoxMac * yBoxMac) - cBombLeft;
    SetStatusPlay;
    UpdateBombCount(0);
    AdjustWindow(fAdjust);
}

#define fValidStep(x,y) (!(fVISIT(x,y) || fGUESSBOMB(x,y)))

static void PushBoxDown(INT x, INT y)
{
    BLK iBlk = iBLK(x, y);

    if (iBlk == iBlkGuessUp)
        iBlk = iBlkGuessDn;
    else if (iBlk == iBlkBlankUp)
        iBlk = iBlkBlank;

    SetBlk(x, y, iBlk);
}

static void PopBoxUp(INT x, INT y)
{
    BLK iBlk = iBLK(x, y);

    if (iBlk == iBlkGuessDn)
        iBlk = iBlkGuessUp;
    else if (iBlk == iBlkBlank)
        iBlk = iBlkBlankUp;

    SetBlk(x, y, iBlk);
}

void TrackMouse(INT xNew, INT yNew)
{
    INT xOld;
    INT yOld;

    if (xNew == xCur && yNew == yCur)
        return;

    xOld = xCur;
    yOld = yCur;
    xCur = xNew;
    yCur = yNew;

    if (fBlock)
    {
        INT x;
        INT y;
        BOOL fValidNew = fInRange(xNew, yNew);
        BOOL fValidOld = fInRange(xOld, yOld);
        INT yOldMin = max(yOld - 1, 1);
        INT yOldMax = min(yOld + 1, yBoxMac);
        INT yCurMin = max(yCur - 1, 1);
        INT yCurMax = min(yCur + 1, yBoxMac);
        INT xOldMin = max(xOld - 1, 1);
        INT xOldMax = min(xOld + 1, xBoxMac);
        INT xCurMin = max(xCur - 1, 1);
        INT xCurMax = min(xCur + 1, xBoxMac);

        if (fValidOld)
            for (y = yOldMin; y <= yOldMax; ++y)
                for (x = xOldMin; x <= xOldMax; ++x)
                    if (!fVISIT(x, y))
                        PopBoxUp(x, y);

        if (fValidNew)
            for (y = yCurMin; y <= yCurMax; ++y)
                for (x = xCurMin; x <= xCurMax; ++x)
                    if (!fVISIT(x, y))
                        PushBoxDown(x, y);

        if (fValidOld)
            for (y = yOldMin; y <= yOldMax; ++y)
                for (x = xOldMin; x <= xOldMax; ++x)
                    DisplayBlk(x, y);

        if (fValidNew)
            for (y = yCurMin; y <= yCurMax; ++y)
                for (x = xCurMin; x <= xCurMax; ++x)
                    DisplayBlk(x, y);
    }
    else
    {
        if (fInRange(xOld, yOld) && !fVISIT(xOld, yOld))
        {
            PopBoxUp(xOld, yOld);
            DisplayBlk(xOld, yOld);
        }

        if (fInRange(xNew, yNew) && fValidStep(xNew, yNew))
        {
            PushBoxDown(xNew, yNew);
            DisplayBlk(xNew, yNew);
        }
    }
}

void MakeGuess(INT x, INT y)
{
    BLK iBlk;

    if (!fInRange(x, y) || fVISIT(x, y))
        return;

    if (fGUESSBOMB(x, y))
    {
        iBlk = Preferences.fMark ? iBlkGuessUp : iBlkBlankUp;
        UpdateBombCount(+1);
    }
    else if (fGUESSMARK(x, y))
    {
        iBlk = iBlkBlankUp;
    }
    else
    {
        iBlk = iBlkBombUp;
        UpdateBombCount(-1);
    }

    ChangeBlk(x, y, iBlk);

    if (fGUESSBOMB(x, y) && fCheckWin())
        GameOver(fWin);
}

void DoButton1Up(void)
{
    if (fInRange(xCur, yCur))
    {
        if (cBoxVisit == 0 && cSec == 0)
        {
            PlayTune(TUNE_TICK);
            ++cSec;
            DisplayTime();
            fTimer = TRUE;
        }

        if (!fStatusPlay)
            xCur = yCur = -2;

        if (fBlock)
            StepBlock(xCur, yCur);
        else if (fValidStep(xCur, yCur))
            StepSquare(xCur, yCur);
    }

    DisplayButton(iButtonCur);
}

void PauseGame(void)
{
    KillTune();
    if (fStatusPlay)
        fTimer = FALSE;
    SetStatusPause;
}

void ResumeGame(void)
{
    if (fStatusPlay)
        fTimer = TRUE;
    ClrStatusPause;
}

void UpdateBombCount(INT BombAdjust)
{
    cBombLeft += BombAdjust;
    DisplayBombCount();
}
