#ifndef WINMINE_MAIN_H
#define WINMINE_MAIN_H

#ifdef DEBUG
#define CHEAT
#endif

#define fTrue TRUE
#define fFalse FALSE
#define hNil NULL

#ifndef STRETCH_COLORONCOLOR
#define STRETCH_COLORONCOLOR COLORONCOLOR
#endif

#define cchMsgMax 128
#define cchMaxPathname MAX_PATH

#define ID_TIMER 1

#define fmenuAlwaysOn 0x00
#define fmenuOff      0x01
#define fmenuOn       0x02

#define FMenuSwitchable()   (Preferences.fMenu != fmenuAlwaysOn)
#define FMenuOn()           ((Preferences.fMenu & 0x01) == 0)

#define fCalc    0x01
#define fResize  0x02
#define fDisplay 0x04

#define FloorDiv(a, b) (((a) >= 0) ? ((a) / (b)) : (-(((-(a)) + (b) - 1) / (b))))
#define xBoxFromXpos(x) FloorDiv((x) - (dxGridOff - dxBlk), dxBlk)
#define yBoxFromYpos(y) FloorDiv((y) - (dyGridOff - dyBlk), dyBlk)

extern INT dypCaption;
extern INT dypMenu;
extern INT dxpBorder;

void AdjustWindow(int fAdjust);
void FixMenus(void);
void DoEnterName(void);
void DoDisplayBest(void);

#endif
