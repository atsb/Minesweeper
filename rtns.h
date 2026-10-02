#ifndef WINMINE_RTNS_H
#define WINMINE_RTNS_H

#define iBlkBlank    0
#define iBlk1        1
#define iBlk2        2
#define iBlk8        8
#define iBlkGuessDn  9
#define iBlkBombDn   10
#define iBlkWrong    11
#define iBlkExplode  12
#define iBlkGuessUp  13
#define iBlkBombUp   14
#define iBlkBlankUp  15

#define iBlkMax 16

#define MaskBomb  0x80
#define MaskVisit 0x40
#define MaskFlags 0xE0
#define MaskData  0x1F
#define NOTMaskBomb 0x7F

#define cBlkMax (27 * 32)

#define iLed0        0
#define iLed1        1
#define iLed9        9
#define iLedBlank   10
#define iLedNegative 11
#define iLedMax 12

#define iButtonHappy   0
#define iButtonCaution 1
#define iButtonLose    2
#define iButtonWin     3
#define iButtonDown    4
#define iButtonMax 5

#define wGameBegin  0
#define wGameInter  1
#define wGameExpert 2
#define wGameOther  3

#define fInRange(x,y) (((x) > 0) && ((y) > 0) && ((x) <= xBoxMac) && ((y) <= yBoxMac))
#define IBLK(x,y) (rgBlk[((y) << 5) + (x)])
#define iBLK(x,y) (IBLK(x,y) & MaskData)
#define SetBorder(x,y) (IBLK(x,y) = iBlkMax)
#define fBORDER(x,y) (IBLK(x,y) == iBlkMax)
#define SetBomb(x,y) (IBLK(x,y) |= MaskBomb)
#define ClearBomb(x,y) (IBLK(x,y) &= NOTMaskBomb)
#define fISBOMB(x,y) ((IBLK(x,y) & MaskBomb) != 0)
#define SetVisit(x,y) (IBLK(x,y) |= MaskVisit)
#define fVISIT(x,y) ((IBLK(x,y) & MaskVisit) != 0)
#define fGUESSBOMB(x,y) (iBLK(x,y) == iBlkBombUp)
#define fGUESSMARK(x,y) (iBLK(x,y) == iBlkGuessUp)
#define SetBlk(x,y,blk) (IBLK(x,y) = (BYTE)(((IBLK(x,y) & MaskFlags) | (blk))))

#define fPlay  0x01
#define fPause 0x02
#define fPanic 0x04
#define fIcon  0x08
#define fDemo  0x10

#define fStatusIcon  (fStatus & fIcon)
#define fStatusPlay  (fStatus & fPlay)
#define fStatusPanic (fStatus & fPanic)
#define fStatusPause (fStatus & fPause)
#define fStatusDemo  (fStatus & fDemo)

#define SetStatusPlay  (fStatus = fPlay)
#define SetStatusPause (fStatus |= fPause)
#define SetStatusPanic (fStatus |= fPanic)
#define SetStatusIcon  (fStatus |= fIcon)
#define SetStatusDemo  (fStatus = fDemo)

#define ClrStatusPlay  (fStatus &= ~fPlay)
#define ClrStatusPause (fStatus &= ~fPause)
#define ClrStatusPanic (fStatus &= ~fPanic)
#define ClrStatusIcon  (fStatus &= ~fIcon)
#define ClrStatusDemo  (fStatus &= ~fDemo)

#define fLose fFalse
#define fWin fTrue

typedef BYTE BLK;

void StartGame(void);
void DoTimer(void);
void TrackMouse(INT, INT);
void DoButton1Up(void);
void MakeGuess(INT, INT);
void PauseGame(void);
void ResumeGame(void);
void ClearField(void);
void UpdateBombCount(INT);

#endif
