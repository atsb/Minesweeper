#ifndef WINMINE_GRAFIX_H
#define WINMINE_GRAFIX_H

#define DXBLK_BASE 16
#define DYBLK_BASE 16
#define DXLED_BASE 13
#define DYLED_BASE 23
#define DXBUTTON_BASE 24
#define DYBUTTON_BASE 24
#define DXFUDGE_BASE 2
#define DXLEFTSPACE_BASE 12
#define DXRIGHTSPACE_BASE 12
#define DYTOPSPACE_BASE 12
#define DYBOTTOMSPACE_BASE 12
#define DYGRIDGAP_BASE 16
#define DYTOPLEDGAP_BASE 4
#define DXLEFTBOMB_OFFSET_BASE 5
#define DXRIGHTTIME_OFFSET_BASE 5

extern INT dxBlk;
extern INT dyBlk;
extern INT dxLed;
extern INT dyLed;
extern INT dxButton;
extern INT dyButton;
extern INT dxFudge;
extern INT dxLeftSpace;
extern INT dxRightSpace;
extern INT dyTopSpace;
extern INT dyBottomSpace;
extern INT dxGridOff;
extern INT dyGridOff;
extern INT dxLeftBomb;
extern INT dxRightTime;
extern INT dyTopLed;

INT DpiScale(INT value);
void SetGraphicsDpi(UINT dpi);
UINT GetGraphicsDpi(void);

#ifdef DEBUG
#define Oops(szMsg) MessageBoxA(NULL, (szMsg), "Oops", MB_OK | MB_ICONHAND)
#else
#define Oops(szMsg) ((void)0)
#endif

BOOL FInitLocal(void);
void CleanUp(void);
void DrawBlk(HDC, INT, INT);
void DisplayBlk(INT, INT);
void DrawButton(HDC, INT);
void DisplayButton(INT);
void DrawGrid(HDC);
void DisplayGrid(void);
void DrawBombCount(HDC);
void DisplayBombCount(void);
void DrawTime(HDC);
void DisplayTime(void);
void DrawScreen(HDC);
void DisplayScreen(void);
BOOL FLoadBitmaps(void);
void FreeBitmaps(void);

#endif
