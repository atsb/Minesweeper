#include <windows.h>
#include "main.h"
#include "res.h"
#include "grafix.h"
#include "rtns.h"
#include "pref.h"
#include "sound.h"

static INT rgDibOff[iBlkMax];
static INT rgDibLedOff[iLedMax];
static INT rgDibButton[iButtonMax];

static BYTE *lpDibBlks;
static BYTE *lpDibLed;
static BYTE *lpDibButton;

static HGLOBAL hResBlks;
static HGLOBAL hResLed;
static HGLOBAL hResButton;
static HRSRC hResInfoBlks;
static HRSRC hResInfoLed;
static HRSRC hResInfoButton;

static HPEN hGrayPen;
static BOOL fGrayPenOwned;

static HDC MemBlkDc[iBlkMax];
static HBITMAP MemBlkBitmap[iBlkMax];

INT dxBlk = DXBLK_BASE;
INT dyBlk = DYBLK_BASE;
INT dxLed = DXLED_BASE;
INT dyLed = DYLED_BASE;
INT dxButton = DXBUTTON_BASE;
INT dyButton = DYBUTTON_BASE;
INT dxFudge = DXFUDGE_BASE;
INT dxLeftSpace = DXLEFTSPACE_BASE;
INT dxRightSpace = DXRIGHTSPACE_BASE;
INT dyTopSpace = DYTOPSPACE_BASE;
INT dyBottomSpace = DYBOTTOMSPACE_BASE;
INT dxGridOff = DXLEFTSPACE_BASE;
INT dyGridOff = DYTOPSPACE_BASE + DYTOPLEDGAP_BASE + DYLED_BASE + DYGRIDGAP_BASE;
INT dxLeftBomb = DXLEFTSPACE_BASE + DXLEFTBOMB_OFFSET_BASE;
INT dxRightTime = DXRIGHTSPACE_BASE + DXRIGHTTIME_OFFSET_BASE;
INT dyTopLed = DYTOPSPACE_BASE + DYTOPLEDGAP_BASE;

static UINT g_dpi = 96;

INT DpiScale(INT value)
{
    return MulDiv(value, (INT)g_dpi, 96);
}

void SetGraphicsDpi(UINT dpi)
{
    if (dpi == 0)
        dpi = 96;

    g_dpi = dpi;
    dxBlk = DpiScale(DXBLK_BASE);
    dyBlk = DpiScale(DYBLK_BASE);
    dxLed = DpiScale(DXLED_BASE);
    dyLed = DpiScale(DYLED_BASE);
    dxButton = DpiScale(DXBUTTON_BASE);
    dyButton = DpiScale(DYBUTTON_BASE);
    dxFudge = DpiScale(DXFUDGE_BASE);
    dxLeftSpace = DpiScale(DXLEFTSPACE_BASE);
    dxRightSpace = DpiScale(DXRIGHTSPACE_BASE);
    dyTopSpace = DpiScale(DYTOPSPACE_BASE);
    dyBottomSpace = DpiScale(DYBOTTOMSPACE_BASE);
    dxGridOff = dxLeftSpace;
    dyTopLed = dyTopSpace + DpiScale(DYTOPLEDGAP_BASE);
    dyGridOff = dyTopLed + dyLed + DpiScale(DYGRIDGAP_BASE);
    dxLeftBomb = dxLeftSpace + DpiScale(DXLEFTBOMB_OFFSET_BASE);
    dxRightTime = dxRightSpace + DpiScale(DXRIGHTTIME_OFFSET_BASE);
}

UINT GetGraphicsDpi(void)
{
    return g_dpi;
}

extern HWND hwndMain;
extern HINSTANCE hInst;
extern PREF Preferences;
extern BYTE rgBlk[cBlkMax];
extern INT dxWindow;
extern INT dyWindow;
extern INT fStatus;
extern INT cSec;
extern INT cBombLeft;
extern INT cBoxVisit;
extern INT xBoxMac;
extern INT yBoxMac;
extern INT iButtonCur;
extern BOOL fEGA;

BOOL FInitLocal(void)
{
    if (!FLoadBitmaps())
        return FALSE;

    ClearField();
    return TRUE;
}

static HRSRC HFindBitmap(INT id)
{
    return FindResourceA(hInst,
                          MAKEINTRESOURCEA(id + (!Preferences.fColor)),
                          RT_BITMAP);
}

static INT CbBitmap(INT x, INT y)
{
    x *= Preferences.fColor ? 4 : 1;
    return y * (((x + 31) >> 5) << 2);
}

BOOL FLoadBitmaps(void)
{
    HDC hDC;
    INT i;
    INT cbDibHeader;
    INT cb;

    hResInfoBlks = HFindBitmap(ID_BMP_BLOCKS);
    hResInfoLed = HFindBitmap(ID_BMP_LED);
    hResInfoButton = HFindBitmap(ID_BMP_BUTTON);

    if (hResInfoBlks == NULL || hResInfoLed == NULL || hResInfoButton == NULL)
        return FALSE;

    hResBlks = LoadResource(hInst, hResInfoBlks);
    hResLed = LoadResource(hInst, hResInfoLed);
    hResButton = LoadResource(hInst, hResInfoButton);

    if (hResBlks == NULL || hResLed == NULL || hResButton == NULL)
    {
        FreeBitmaps();
        return FALSE;
    }

    lpDibBlks = (BYTE *)LockResource(hResBlks);
    lpDibLed = (BYTE *)LockResource(hResLed);
    lpDibButton = (BYTE *)LockResource(hResButton);

    if (lpDibBlks == NULL || lpDibLed == NULL || lpDibButton == NULL)
    {
        FreeBitmaps();
        return FALSE;
    }

    hGrayPen = NULL;
    fGrayPenOwned = FALSE;

    if (!Preferences.fColor)
    {
        hGrayPen = (HPEN)GetStockObject(BLACK_PEN);
    }
    else if (fEGA)
    {
        BITMAPINFO *biBlks = (BITMAPINFO *)lpDibBlks;
        BITMAPINFO *biButton = (BITMAPINFO *)lpDibButton;

        biBlks->bmiColors[8].rgbBlue = 0x80;
        biBlks->bmiColors[8].rgbGreen = 0x80;
        biBlks->bmiColors[8].rgbRed = 0x80;
        biBlks->bmiColors[8].rgbReserved = 0;
        biButton->bmiColors[8].rgbBlue = 0x80;
        biButton->bmiColors[8].rgbGreen = 0x80;
        biButton->bmiColors[8].rgbRed = 0x80;
        biButton->bmiColors[8].rgbReserved = 0;
        biBlks->bmiColors[7].rgbBlue = 0x40;
        biBlks->bmiColors[7].rgbGreen = 0x40;
        biBlks->bmiColors[7].rgbRed = 0x40;
        biBlks->bmiColors[7].rgbReserved = 0;
        biButton->bmiColors[7].rgbBlue = 0x40;
        biButton->bmiColors[7].rgbGreen = 0x40;
        biButton->bmiColors[7].rgbRed = 0x40;
        biButton->bmiColors[7].rgbReserved = 0;

        hGrayPen = CreatePen(PS_SOLID, 1, RGB(64, 64, 64));
        fGrayPenOwned = hGrayPen != NULL;
    }
    else
    {
        hGrayPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
        fGrayPenOwned = hGrayPen != NULL;
    }

    cbDibHeader = sizeof(BITMAPINFOHEADER) + (Preferences.fColor ? 16 * 4 : 2 * 4);

    cb = CbBitmap(DXBLK_BASE, DYBLK_BASE);
    for (i = 0; i < iBlkMax; ++i)
        rgDibOff[i] = cbDibHeader + (i * cb);

    cb = CbBitmap(DXLED_BASE, DYLED_BASE);
    for (i = 0; i < iLedMax; ++i)
        rgDibLedOff[i] = cbDibHeader + (i * cb);

    cb = CbBitmap(DXBUTTON_BASE, DYBUTTON_BASE);
    for (i = 0; i < iButtonMax; ++i)
        rgDibButton[i] = cbDibHeader + (i * cb);

    hDC = GetDC(hwndMain);
    if (hDC == NULL)
    {
        FreeBitmaps();
        return FALSE;
    }

    for (i = 0; i < iBlkMax; ++i)
    {
        MemBlkDc[i] = CreateCompatibleDC(hDC);
        if (MemBlkDc[i] == NULL)
        {
            ReleaseDC(hwndMain, hDC);
            FreeBitmaps();
            return FALSE;
        }

        MemBlkBitmap[i] = CreateCompatibleBitmap(hDC, dxBlk, dyBlk);
        if (MemBlkBitmap[i] == NULL)
        {
            ReleaseDC(hwndMain, hDC);
            FreeBitmaps();
            return FALSE;
        }

        SelectObject(MemBlkDc[i], MemBlkBitmap[i]);

        SetStretchBltMode(MemBlkDc[i], STRETCH_COLORONCOLOR);
        if (StretchDIBits(MemBlkDc[i],
                          0, 0, dxBlk, dyBlk,
                          0, 0, DXBLK_BASE, DYBLK_BASE,
                          lpDibBlks + rgDibOff[i],
                          (BITMAPINFO *)lpDibBlks,
                          DIB_RGB_COLORS,
                          SRCCOPY) == GDI_ERROR)
        {
            ReleaseDC(hwndMain, hDC);
            FreeBitmaps();
            return FALSE;
        }
    }

    ReleaseDC(hwndMain, hDC);
    return TRUE;
}

void FreeBitmaps(void)
{
    INT i;

    if (fGrayPenOwned && hGrayPen != NULL)
        DeleteObject(hGrayPen);
    hGrayPen = NULL;
    fGrayPenOwned = FALSE;

    for (i = 0; i < iBlkMax; ++i)
    {
        if (MemBlkDc[i] != NULL)
            DeleteDC(MemBlkDc[i]);
        if (MemBlkBitmap[i] != NULL)
            DeleteObject(MemBlkBitmap[i]);
        MemBlkDc[i] = NULL;
        MemBlkBitmap[i] = NULL;
    }

    hResBlks = NULL;
    hResLed = NULL;
    hResButton = NULL;
    hResInfoBlks = NULL;
    hResInfoLed = NULL;
    hResInfoButton = NULL;
    lpDibBlks = NULL;
    lpDibLed = NULL;
    lpDibButton = NULL;
}

void CleanUp(void)
{
    FreeBitmaps();
    EndTunes();
}

void DrawBlk(HDC hDC, INT x, INT y)
{
    BitBlt(hDC,
           x * dxBlk + (dxGridOff - dxBlk),
           y * dyBlk + (dyGridOff - dyBlk),
           dxBlk,
           dyBlk,
           MemBlkDc[iBLK(x, y)],
           0,
           0,
           SRCCOPY);
}

void DisplayBlk(INT x, INT y)
{
    HDC hDC = GetDC(hwndMain);
    if (hDC == NULL)
        return;

    DrawBlk(hDC, x, y);
    ReleaseDC(hwndMain, hDC);
}

void DrawGrid(HDC hDC)
{
    INT x;
    INT y;
    INT dx;
    INT dy = dyGridOff;

    for (y = 1; y <= yBoxMac; ++y, dy += dyBlk)
    {
        dx = dxGridOff;
        for (x = 1; x <= xBoxMac; ++x, dx += dxBlk)
        {
            BitBlt(hDC,
                   dx,
                   dy,
                   dxBlk,
                   dyBlk,
                   MemBlkDc[iBLK(x, y)],
                   0,
                   0,
                   SRCCOPY);
        }
    }
}

void DisplayGrid(void)
{
    HDC hDC = GetDC(hwndMain);
    if (hDC == NULL)
        return;
    DrawGrid(hDC);
    ReleaseDC(hwndMain, hDC);
}

static void DrawLed(HDC hDC, INT x, INT iLed)
{
    SetStretchBltMode(hDC, STRETCH_COLORONCOLOR);
    StretchDIBits(hDC,
                  x,
                  dyTopLed,
                  dxLed,
                  dyLed,
                  0,
                  0,
                  DXLED_BASE,
                  DYLED_BASE,
                  lpDibLed + rgDibLedOff[iLed],
                  (BITMAPINFO *)lpDibLed,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

void DrawBombCount(HDC hDC)
{
    INT iLed;
    INT cBombs;

    if (cBombLeft < 0)
    {
        iLed = iLedNegative;
        cBombs = (-cBombLeft) % 100;
    }
    else
    {
        iLed = cBombLeft / 100;
        cBombs = cBombLeft % 100;
    }

    DrawLed(hDC, dxLeftBomb, iLed);
    DrawLed(hDC, dxLeftBomb + dxLed, cBombs / 10);
    DrawLed(hDC, dxLeftBomb + 2 * dxLed, cBombs % 10);
}

void DisplayBombCount(void)
{
    HDC hDC = GetDC(hwndMain);
    if (hDC == NULL)
        return;
    DrawBombCount(hDC);
    ReleaseDC(hwndMain, hDC);
}

void DrawTime(HDC hDC)
{
    INT iLed = cSec;

    DrawLed(hDC, dxWindow - (dxRightTime + 3 * dxLed + dxpBorder), iLed / 100);
    iLed %= 100;
    DrawLed(hDC, dxWindow - (dxRightTime + 2 * dxLed + dxpBorder), iLed / 10);
    DrawLed(hDC, dxWindow - (dxRightTime + dxLed + dxpBorder), iLed % 10);
}

void DisplayTime(void)
{
    HDC hDC = GetDC(hwndMain);
    if (hDC == NULL)
        return;
    DrawTime(hDC);
    ReleaseDC(hwndMain, hDC);
}

void DrawButton(HDC hDC, INT iButton)
{
    SetStretchBltMode(hDC, STRETCH_COLORONCOLOR);
    StretchDIBits(hDC,
                  (dxWindow - dxButton) >> 1,
                  dyTopLed,
                  dxButton,
                  dyButton,
                  0,
                  0,
                  DXBUTTON_BASE,
                  DYBUTTON_BASE,
                  lpDibButton + rgDibButton[iButton],
                  (BITMAPINFO *)lpDibButton,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

void DisplayButton(INT iButton)
{
    HDC hDC = GetDC(hwndMain);
    if (hDC == NULL)
        return;
    DrawButton(hDC, iButton);
    ReleaseDC(hwndMain, hDC);
}

static void SetThePen(HDC hDC, INT fNormal)
{
    if ((fNormal & 1) != 0)
    {
        SetROP2(hDC, R2_WHITE);
    }
    else
    {
        SetROP2(hDC, R2_COPYPEN);
        if (hGrayPen != NULL)
            SelectObject(hDC, hGrayPen);
    }
}

static void DrawBorder(HDC hDC, INT x1, INT y1, INT x2, INT y2, INT width, INT fNormal)
{
    INT i = 0;

    SetThePen(hDC, fNormal);

    while (i++ < width)
    {
        MoveToEx(hDC, x1, --y2, NULL);
        LineTo(hDC, x1++, y1);
        LineTo(hDC, x2--, y1++);
    }

    if (fNormal < 2)
        SetThePen(hDC, fNormal ^ 1);

    while (--i)
    {
        MoveToEx(hDC, x1--, ++y2, NULL);
        LineTo(hDC, ++x2, y2);
        LineTo(hDC, x2, --y1);
    }
}

static void DrawBackground(HDC hDC)
{
    INT x;
    INT y;

    INT b1 = DpiScale(1);
    INT b2 = DpiScale(2);
    INT b3 = DpiScale(3);
    DrawBorder(hDC, 0, 0, x = dxWindow - b1, y = dyWindow - b1, b3, 1);
    DrawBorder(hDC, dxGridOff - b3, dyGridOff - b3,
               x -= dxRightSpace - b3, y - (dyBottomSpace - b3), b3, 0);
    DrawBorder(hDC, dxGridOff - b3, dyTopSpace - b3,
               x, dyTopLed + dyLed + (dyBottomSpace - DpiScale(6)), b2, 0);
    DrawBorder(hDC, dxLeftBomb - b1, dyTopLed - b1,
               dxLeftBomb + dxLed * 3, y = dyTopLed + dyLed, b1, 0);
    DrawBorder(hDC,
               x = dxWindow - (dxRightTime + 3 * dxLed + dxpBorder + b1),
               dyTopLed - b1,
               x + dxLed * 3 + b1,
               y,
               b1,
               0);
    DrawBorder(hDC,
               x = ((dxWindow - dxButton) >> 1) - b1,
               dyTopLed - b1,
               x + dxButton + b1,
               dyTopLed + dyButton,
               b1,
               2);
}

void DrawScreen(HDC hDC)
{
    DrawBackground(hDC);
    DrawBombCount(hDC);
    DrawButton(hDC, iButtonCur);
    DrawTime(hDC);
    DrawGrid(hDC);
}

void DisplayScreen(void)
{
    HDC hDC = GetDC(hwndMain);
    if (hDC == NULL)
        return;
    DrawScreen(hDC);
    ReleaseDC(hwndMain, hDC);
}
