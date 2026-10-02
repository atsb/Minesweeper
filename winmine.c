#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include "main.h"
#include "rtns.h"
#include "grafix.h"
#include "res.h"
#include "pref.h"
#include "util.h"
#include "sound.h"
#include "context.h"

BOOL bInitMinimized;
HINSTANCE hInst;
HWND hwndMain;
HMENU hMenu;
BOOL fButton1Down = FALSE;
BOOL fBlock = FALSE;
BOOL fIgnoreClick = FALSE;
INT dypCaption;
INT dypMenu;
INT dxpBorder;
INT fStatus = fDemo | fIcon;
BOOL fLocalPause = FALSE;
BOOL fEGA;
CHAR szClass[cchNameMax];
#define szWindowTitle szClass
CHAR szTime[cchNameMax];
CHAR szDefaultName[cchNameMax];

extern BOOL fUpdateIni;
extern INT xCur;
extern INT yCur;
extern INT iButtonCur;
extern INT xBoxMac;
extern INT yBoxMac;
extern PREF Preferences;

INT dxWindow;
INT dyWindow;
INT dypAdjust;

#define iPrefMax 3
#define idRadMax 3

static INT rgLevelData[idRadMax][iPrefMax] =
{
    {10, 8, 8},
    {40, 16, 16},
    {99, 16, 30}
};

#ifndef DEBUG
#define XYZZY
#define cchXYZZY 5
static INT iXYZZY = 0;
static CHAR szXYZZY[cchXYZZY] = "XYZZY";
extern BYTE rgBlk[cBlkMax];
#endif

static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK PrefDlgProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK BestDlgProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK EnterDlgProc(HWND, UINT, WPARAM, LPARAM);

static DWORD MainWindowStyle(void)
{
    return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
}

static BOOL GetMainWindowFrame(RECT *rect)
{
    HMENU menu;

    if (rect == NULL)
        return FALSE;

    SetRect(rect, 0, 0, dxWindow, dyWindow);
    menu = FMenuOn() && hMenu != NULL ? hMenu : NULL;
    if (AdjustWindowRectExForDpi(rect, MainWindowStyle(), menu != NULL, 0, GetGraphicsDpi()) == FALSE)
        return FALSE;

    return TRUE;
}

static void EnableHighDpi(void)
{
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        SetProcessDPIAware();
}

static BOOL FLocalButton(LPARAM lParam)
{
    BOOL fDown = TRUE;
    RECT rcCapt;
    MSG msg;
    POINT pt;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    rcCapt.left = (dxWindow - dxButton) >> 1;
    rcCapt.top = dyTopLed;
    rcCapt.right = rcCapt.left + dxButton;
    rcCapt.bottom = rcCapt.top + dyButton;

    if (!PtInRect(&rcCapt, pt))
        return FALSE;

    SetCapture(hwndMain);
    DisplayButton(iButtonDown);

    {
        POINT ptLeftTop;
        POINT ptRightBottom;
        ptLeftTop.x = rcCapt.left;
        ptLeftTop.y = rcCapt.top;
        ptRightBottom.x = rcCapt.right;
        ptRightBottom.y = rcCapt.bottom;
        ClientToScreen(hwndMain, &ptLeftTop);
        ClientToScreen(hwndMain, &ptRightBottom);
        rcCapt.left = ptLeftTop.x;
        rcCapt.top = ptLeftTop.y;
        rcCapt.right = ptRightBottom.x;
        rcCapt.bottom = ptRightBottom.y;
    }

    while (TRUE)
    {
        if (PeekMessage(&msg, hwndMain, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        {
            switch (msg.message)
            {
                case WM_LBUTTONUP:
                    if (fDown && PtInRect(&rcCapt, msg.pt))
                    {
                        DisplayButton(iButtonCur = iButtonHappy);
                        StartGame();
                    }
                    ReleaseCapture();
                    return TRUE;

                case WM_MOUSEMOVE:
                    if (PtInRect(&rcCapt, msg.pt))
                    {
                        if (!fDown)
                        {
                            fDown = TRUE;
                            DisplayButton(iButtonDown);
                        }
                    }
                    else if (fDown)
                    {
                        fDown = FALSE;
                        DisplayButton(iButtonCur);
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

void FixMenus(void)
{
    CheckEm(IDM_BEGIN, Preferences.wGameType == wGameBegin);
    CheckEm(IDM_INTER, Preferences.wGameType == wGameInter);
    CheckEm(IDM_EXPERT, Preferences.wGameType == wGameExpert);
    CheckEm(IDM_CUSTOM, Preferences.wGameType == wGameOther);
    CheckEm(IDM_COLOR, Preferences.fColor);
    CheckEm(IDM_MARK, Preferences.fMark);
#ifdef NOISEY
    CheckEm(IDM_SOUND, Preferences.fSound);
#endif
}

void DoPref(void)
{
    DialogBoxA(hInst, MAKEINTRESOURCEA(ID_DLG_PREF), hwndMain, PrefDlgProc);
    Preferences.wGameType = wGameOther;
    FixMenus();
    fUpdateIni = TRUE;
    StartGame();
}

void DoEnterName(void)
{
    DialogBoxA(hInst, MAKEINTRESOURCEA(ID_DLG_ENTER), hwndMain, EnterDlgProc);
    fUpdateIni = TRUE;
}

void DoDisplayBest(void)
{
    DialogBoxA(hInst, MAKEINTRESOURCEA(ID_DLG_BEST), hwndMain, BestDlgProc);
}

static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_MOVE:
            if (!fStatusIcon)
            {
                POINT pt;
                pt.x = 0;
                pt.y = 0;
                if (ClientToScreen(hWnd, &pt))
                {
                    Preferences.xWindow = pt.x;
                    Preferences.yWindow = pt.y;
                }
            }
            break;

        case WM_EXITSIZEMOVE:
        {
            POINT pt;
            pt.x = 0;
            pt.y = 0;
            if (!fStatusIcon && ClientToScreen(hWnd, &pt))
            {
                Preferences.xWindow = pt.x;
                Preferences.yWindow = pt.y;
                fUpdateIni = TRUE;
            }
            break;
        }

        case WM_DPICHANGED:
            AdjustWindow(fResize | fDisplay);
            return 0;

        case WM_NCLBUTTONDOWN:
            if (wParam == HTCAPTION)
            {
                ReleaseCapture();
                SendMessageA(hWnd, WM_SYSCOMMAND, (WPARAM)(SC_MOVE | HTCAPTION), 0);
                return 0;
            }
            break;

        case WM_SYSCOMMAND:
            switch (wParam & 0xFFF0)
            {
                case SC_MINIMIZE:
                    PauseGame();
                    SetStatusPause;
                    SetStatusIcon;
                    break;

                case SC_RESTORE:
                    ClrStatusPause;
                    ClrStatusIcon;
                    ResumeGame();
                    fIgnoreClick = FALSE;
                    break;

                default:
                    break;
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_NEW:
                    StartGame();
                    break;

                case IDM_EXIT:
                    ShowWindow(hwndMain, SW_HIDE);
                    SendMessageA(hwndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
                    return 0;

                case IDM_BEGIN:
                case IDM_INTER:
                case IDM_EXPERT:
                    Preferences.wGameType = (INT)(LOWORD(wParam) - IDM_BEGIN);
                    Preferences.Mines = rgLevelData[Preferences.wGameType][0];
                    Preferences.Height = rgLevelData[Preferences.wGameType][1];
                    Preferences.Width = rgLevelData[Preferences.wGameType][2];
                    StartGame();
                    fUpdateIni = TRUE;
                    FixMenus();
                    break;

                case IDM_CUSTOM:
                    DoPref();
                    break;

#ifdef NOISEY
                case IDM_SOUND:
                    if (Preferences.fSound)
                    {
                        EndTunes();
                        Preferences.fSound = fsoundOff;
                    }
                    else
                    {
                        Preferences.fSound = FInitTunes();
                    }
                    fUpdateIni = TRUE;
                    FixMenus();
                    break;
#endif

                case IDM_COLOR:
                    Preferences.fColor = !Preferences.fColor;
                    FreeBitmaps();
                    if (!FLoadBitmaps())
                    {
                        ReportErr(ID_ERR_MEM);
                        SendMessageA(hwndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
                        return 0;
                    }
                    DisplayScreen();
                    fUpdateIni = TRUE;
                    FixMenus();
                    break;

                case IDM_MARK:
                    Preferences.fMark = !Preferences.fMark;
                    fUpdateIni = TRUE;
                    FixMenus();
                    break;

                case IDM_BEST:
                    DoDisplayBest();
                    break;

                case IDM_INDEX:
                    DoHelp(HELP_INDEX, 0);
                    break;

                case IDM_HOW2PLAY:
                    DoHelp(HELP_CONTEXT, HLP_HOWTOPLAY);
                    break;

                case IDM_COMMANDS:
                    DoHelp(HELP_CONTEXT, HLP_COMMANDS);
                    break;

                case IDM_HELP_HELP:
                    DoHelp(HELP_HELPONHELP, 0);
                    break;

                case IDM_HELP_ABOUT:
                    DoAbout();
                    return 0;

                default:
                    break;
            }
            break;

        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_ESCAPE:
                    SetStatusPanic;
                    PostMessageA(hwndMain, WM_SYSCOMMAND, SC_MINIMIZE, 0);
                    break;

                case VK_F4:
                    if (FSoundSwitchable())
                    {
                        if (FSoundOn())
                        {
                            EndTunes();
                            Preferences.fSound = fsoundOff;
                        }
                        else
                        {
                            Preferences.fSound = FInitTunes();
                        }
                    }
                    break;

                case VK_F5:
                    if (FMenuSwitchable())
                        SetMenuBar(fmenuOff);
                    break;

                case VK_F6:
                    if (FMenuSwitchable())
                        SetMenuBar(fmenuOn);
                    break;

#ifdef XYZZY
                case VK_SHIFT:
                    if (iXYZZY >= cchXYZZY)
                        iXYZZY ^= 20;
                    break;

                default:
                    if (iXYZZY < cchXYZZY)
                        iXYZZY = (szXYZZY[iXYZZY] == (CHAR)wParam) ? iXYZZY + 1 : 0;
                    break;
#else
                default:
                    break;
#endif
            }
            break;

        case WM_DESTROY:
            KillTimer(hwndMain, ID_TIMER);
            PostQuitMessage(0);
            break;

        case WM_MBUTTONDOWN:
            if (fIgnoreClick)
            {
                fIgnoreClick = FALSE;
                return 0;
            }

            if (!fStatusPlay)
                break;

            fBlock = TRUE;
            goto LBigStep;

        case WM_LBUTTONDOWN:
            if (fIgnoreClick)
            {
                fIgnoreClick = FALSE;
                return 0;
            }

            if (FLocalButton(lParam))
                return 0;

            if (!fStatusPlay)
                break;

            fBlock = (wParam & (MK_SHIFT | MK_RBUTTON)) ? TRUE : FALSE;

LBigStep:
            SetCapture(hWnd);
            fButton1Down = TRUE;
            xCur = -1;
            yCur = -1;
            DisplayButton(iButtonCaution);
            goto LMouseMove;

        case WM_MOUSEMOVE:
LMouseMove:
            if (fButton1Down)
            {
                if (fStatusPlay)
                    TrackMouse(xBoxFromXpos(GET_X_LPARAM(lParam)),
                               yBoxFromYpos(GET_Y_LPARAM(lParam)));
                else
                    goto LFixTimeOut;
            }
#ifdef XYZZY
            else if (iXYZZY != 0 &&
                     (((iXYZZY == cchXYZZY) && (wParam & MK_CONTROL)) ||
                      (iXYZZY > cchXYZZY)))
            {
                xCur = xBoxFromXpos(GET_X_LPARAM(lParam));
                yCur = yBoxFromYpos(GET_Y_LPARAM(lParam));
                if (fInRange(xCur, yCur))
                {
                    HDC hDC = GetDC(NULL);
                    if (hDC != NULL)
                    {
                        SetPixel(hDC, 0, 0, fISBOMB(xCur, yCur) ? 0L : 0x00FFFFFFL);
                        ReleaseDC(NULL, hDC);
                    }
                }
            }
#endif
            break;

        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_LBUTTONUP:
            if (fButton1Down)
            {
LFixTimeOut:
                fButton1Down = FALSE;
                ReleaseCapture();
                if (fStatusPlay)
                    DoButton1Up();
                else
                    TrackMouse(-2, -2);
            }
            break;

        case WM_RBUTTONDOWN:
            if (fIgnoreClick)
            {
                fIgnoreClick = FALSE;
                return 0;
            }

            if (!fStatusPlay)
                break;

            if (fButton1Down)
            {
                TrackMouse(-3, -3);
                fBlock = TRUE;
                PostMessageA(hwndMain, WM_MOUSEMOVE, wParam, lParam);
            }
            else if ((wParam & MK_LBUTTON) != 0)
            {
                goto LBigStep;
            }
            else if (!fLocalPause)
            {
                MakeGuess(xBoxFromXpos(GET_X_LPARAM(lParam)),
                          yBoxFromYpos(GET_Y_LPARAM(lParam)));
            }
            return 0;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_CLICKACTIVE)
                fIgnoreClick = TRUE;
            break;

        case WM_TIMER:
#ifdef CHEAT
            if (!fLocalPause)
#endif
                DoTimer();
            return 0;

        case WM_ENTERMENULOOP:
            fLocalPause = TRUE;
            break;

        case WM_EXITMENULOOP:
            fLocalPause = FALSE;
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);
            if (hDC != NULL)
            {
                DrawScreen(hDC);
                EndPaint(hWnd, &ps);
            }
            return 0;
        }

        default:
            break;
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}

static INT_PTR CALLBACK PrefDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
            SetDlgItemInt(hDlg, ID_EDIT_HEIGHT, (UINT)Preferences.Height, FALSE);
            SetDlgItemInt(hDlg, ID_EDIT_WIDTH, (UINT)Preferences.Width, FALSE);
            SetDlgItemInt(hDlg, ID_EDIT_MINES, (UINT)Preferences.Mines, FALSE);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_BTN_OK:
                case IDOK:
                    Preferences.Height = GetDlgInt(hDlg, ID_EDIT_HEIGHT, 8, fEGA ? 16 : 24);
                    Preferences.Width = GetDlgInt(hDlg, ID_EDIT_WIDTH, 8, 30);
                    Preferences.Mines = GetDlgInt(hDlg, ID_EDIT_MINES, 10,
                                                  min(999, (Preferences.Height - 1) *
                                                           (Preferences.Width - 1)));
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case ID_BTN_CANCEL:
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

static void SetDText(HWND hDlg, INT id, INT time, const CHAR *szName)
{
    CHAR sz[cchNameMax];

    wsprintfA(sz, szTime, time);
    SetDlgItemTextA(hDlg, id, sz);
    SetDlgItemTextA(hDlg, id + 1, szName);
}

static INT_PTR CALLBACK BestDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
            SetDText(hDlg, ID_TIME_BEGIN, Preferences.rgTime[wGameBegin], Preferences.szBegin);
            SetDText(hDlg, ID_TIME_INTER, Preferences.rgTime[wGameInter], Preferences.szInter);
            SetDText(hDlg, ID_TIME_EXPERT, Preferences.rgTime[wGameExpert], Preferences.szExpert);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_BTN_RESET:
                    Preferences.rgTime[wGameBegin] = 999;
                    Preferences.rgTime[wGameInter] = 999;
                    Preferences.rgTime[wGameExpert] = 999;
                    memcpy(Preferences.szBegin, szDefaultName, sizeof(Preferences.szBegin));
                    memcpy(Preferences.szInter, szDefaultName, sizeof(Preferences.szInter));
                    memcpy(Preferences.szExpert, szDefaultName, sizeof(Preferences.szExpert));
                    fUpdateIni = TRUE;
                    SetDText(hDlg, ID_TIME_BEGIN, Preferences.rgTime[wGameBegin], Preferences.szBegin);
                    SetDText(hDlg, ID_TIME_INTER, Preferences.rgTime[wGameInter], Preferences.szInter);
                    SetDText(hDlg, ID_TIME_EXPERT, Preferences.rgTime[wGameExpert], Preferences.szExpert);
                    return TRUE;

                case ID_BTN_OK:
                case IDOK:
                case ID_BTN_CANCEL:
                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

static INT_PTR CALLBACK EnterDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            CHAR sz[cchMsgMax];
            CHAR *name;

            LoadSz((WORD)(Preferences.wGameType + ID_MSG_BEGIN), sz);
            SetDlgItemTextA(hDlg, ID_TEXT_BEST, sz);

            if (Preferences.wGameType == wGameBegin)
                name = Preferences.szBegin;
            else if (Preferences.wGameType == wGameInter)
                name = Preferences.szInter;
            else
                name = Preferences.szExpert;

            SetDlgItemTextA(hDlg, ID_EDIT_NAME, name);
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_BTN_OK:
                case IDOK:
                case ID_BTN_CANCEL:
                case IDCANCEL:
                {
                    CHAR *name;

                    if (Preferences.wGameType == wGameBegin)
                        name = Preferences.szBegin;
                    else if (Preferences.wGameType == wGameInter)
                        name = Preferences.szInter;
                    else
                        name = Preferences.szExpert;

                    GetDlgItemTextA(hDlg, ID_EDIT_NAME, name, cchNameMax);
                    EndDialog(hDlg, TRUE);
                    return TRUE;
                }

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

static BOOL fGraphicsInitialized = FALSE;

static void GetMonitorWorkArea(HMONITOR hMonitor, RECT *rcWork)
{
    MONITORINFO mi;

    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);

    if (hMonitor != NULL && GetMonitorInfoA(hMonitor, &mi))
    {
        *rcWork = mi.rcWork;
        return;
    }

    rcWork->left = 0;
    rcWork->top = 0;
    rcWork->right = GetSystemMetrics(SM_CXSCREEN);
    rcWork->bottom = GetSystemMetrics(SM_CYSCREEN);
}

static HMONITOR GetMainMonitor(void)
{
    POINT pt;

    if (hwndMain != NULL)
    {
        pt.x = 0;
        pt.y = 0;
        if (ClientToScreen(hwndMain, &pt))
            return MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    }

    if (!FWindowPositionSaved())
    {
        pt.x = GetSystemMetrics(SM_CXSCREEN) >> 1;
        pt.y = GetSystemMetrics(SM_CYSCREEN) >> 1;
        return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    }

    pt.x = Preferences.xWindow;
    pt.y = Preferences.yWindow;
    return MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
}

static BOOL GetFrameForDpi(UINT dpi, INT clientWidth, INT clientHeight, RECT *frame)
{
    HMENU menu;

    if (frame == NULL)
        return FALSE;

    SetRect(frame, 0, 0, clientWidth, clientHeight);
    menu = FMenuOn() && hMenu != NULL ? hMenu : NULL;
    return AdjustWindowRectExForDpi(frame,
                                   MainWindowStyle(),
                                   menu != NULL,
                                   0,
                                   dpi);
}

static BOOL FitsAtDpi(UINT dpi, const RECT *work)
{
    RECT frame;
    INT clientWidth;
    INT clientHeight;
    INT windowWidth;
    INT windowHeight;

    clientWidth = MulDiv(DXBLK_BASE * xBoxMac +
                         DXLEFTSPACE_BASE + DXRIGHTSPACE_BASE,
                         (INT)dpi,
                         96);
    clientHeight = MulDiv(DYBLK_BASE * yBoxMac +
                          DYTOPSPACE_BASE +
                          DYTOPLEDGAP_BASE +
                          DYLED_BASE +
                          DYGRIDGAP_BASE +
                          DYBOTTOMSPACE_BASE,
                          (INT)dpi,
                          96);

    if (!GetFrameForDpi(dpi, clientWidth, clientHeight, &frame))
        return FALSE;

    windowWidth = frame.right - frame.left;
    windowHeight = frame.bottom - frame.top;

    return windowWidth <= (work->right - work->left) &&
           windowHeight <= (work->bottom - work->top);
}

static UINT GetEffectiveDpi(UINT monitorDpi, const RECT *work)
{
    UINT low;
    UINT high;
    UINT best;

    if (monitorDpi == 0)
        monitorDpi = 96;

    if (FitsAtDpi(monitorDpi, work))
        return monitorDpi;

    low = 48;
    high = monitorDpi;
    best = low;

    while (low <= high)
    {
        UINT mid = low + (high - low) / 2;
        if (FitsAtDpi(mid, work))
        {
            best = mid;
            low = mid + 1;
        }
        else
        {
            if (mid == 0)
                break;
            high = mid - 1;
        }
    }

    return best;
}

static BOOL RebuildScaledGraphics(void)
{
    if (!fGraphicsInitialized)
        return TRUE;

    FreeBitmaps();
    if (!FLoadBitmaps())
        return FALSE;

    return TRUE;
}

void AdjustWindow(INT fAdjust)
{
    HMONITOR hMonitor;
    RECT work;
    RECT frame;
    POINT clientOrigin;
    UINT monitorDpi;
    UINT effectiveDpi;
    INT windowWidth;
    INT windowHeight;
    INT left;
    INT top;

    hMonitor = GetMainMonitor();
    GetMonitorWorkArea(hMonitor, &work);

    if (hwndMain != NULL)
    {
        monitorDpi = GetDpiForWindow(hwndMain);
    }
    else
    {
        monitorDpi = GetDpiForSystem();
    }
    if (monitorDpi == 0)
        monitorDpi = 96;

    effectiveDpi = GetEffectiveDpi(monitorDpi, &work);

    if (effectiveDpi != GetGraphicsDpi())
    {
        SetGraphicsDpi(effectiveDpi);
        dxpBorder = DpiScale(2);
        if (!RebuildScaledGraphics())
        {
            ReportErr(ID_ERR_MEM);
            if (hwndMain != NULL)
                DestroyWindow(hwndMain);
            return;
        }
        fAdjust |= fResize | fDisplay;
    }
    else
    {
        dxpBorder = DpiScale(2);
    }

    dxWindow = dxBlk * xBoxMac + dxGridOff + dxRightSpace;
    dyWindow = dyBlk * yBoxMac + dyGridOff + dyBottomSpace;

    if (!GetFrameForDpi(GetGraphicsDpi(), dxWindow, dyWindow, &frame))
        return;

    windowWidth = frame.right - frame.left;
    windowHeight = frame.bottom - frame.top;

    if (hwndMain != NULL)
    {
        clientOrigin.x = 0;
        clientOrigin.y = 0;
        if (ClientToScreen(hwndMain, &clientOrigin))
        {
            Preferences.xWindow = clientOrigin.x;
            Preferences.yWindow = clientOrigin.y;
        }
    }
    else if (!FWindowPositionSaved())
    {
        INT workWidth = work.right - work.left;
        INT workHeight = work.bottom - work.top;

        Preferences.xWindow = work.left + ((workWidth - windowWidth) >> 1) - frame.left;
        Preferences.yWindow = work.top + ((workHeight - windowHeight) >> 1) - frame.top;
    }

    left = Preferences.xWindow + frame.left;
    top = Preferences.yWindow + frame.top;

    if (left < work.left)
    {
        Preferences.xWindow += work.left - left;
        left = work.left;
        fAdjust |= fResize;
    }

    if (top < work.top)
    {
        Preferences.yWindow += work.top - top;
        top = work.top;
        fAdjust |= fResize;
    }

    if (left + windowWidth > work.right)
    {
        Preferences.xWindow -= (left + windowWidth) - work.right;
        left = work.right - windowWidth;
        fAdjust |= fResize;
    }

    if (top + windowHeight > work.bottom)
    {
        Preferences.yWindow -= (top + windowHeight) - work.bottom;
        top = work.bottom - windowHeight;
        fAdjust |= fResize;
    }

    dypAdjust = frame.bottom - dyWindow;

    if (hwndMain != NULL && !bInitMinimized)
    {
        if (fAdjust & fResize)
        {
            SetWindowPos(hwndMain,
                         NULL,
                         left,
                         top,
                         windowWidth,
                         windowHeight,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }

        if (fAdjust & fDisplay)
            InvalidateRect(hwndMain, NULL, TRUE);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    HACCEL hAccel;
    WNDCLASSA wc;
    int messageResult;

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance;
    EnableHighDpi();
    SetGraphicsDpi(GetDpiForSystem());
    InitConst();

    bInitMinimized = (nCmdShow == SW_SHOWMINNOACTIVE) ||
                     (nCmdShow == SW_SHOWMINIMIZED);

    ZeroMemory(&wc, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInst;
    wc.hIcon = LoadIconA(hInst, MAKEINTRESOURCEA(ID_ICON_MAIN));
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = fEGA
        ? CreateSolidBrush(RGB(128, 128, 128))
        : (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszClassName = szClass;

    if (!RegisterClassA(&wc))
        return 0;

    hMenu = LoadMenuA(hInst, MAKEINTRESOURCEA(ID_MENU));
    hAccel = LoadAcceleratorsA(hInst, MAKEINTRESOURCEA(ID_MENU_ACCEL));

    ReadPreferences();
    AdjustWindow(fCalc);

    {
        RECT frame;
        if (!GetMainWindowFrame(&frame))
            return 0;

        hwndMain = CreateWindowExA(0,
                                   szClass,
                                   szWindowTitle,
                                   MainWindowStyle(),
                                   Preferences.xWindow + frame.left,
                                   Preferences.yWindow + frame.top,
                                   frame.right - frame.left,
                                   frame.bottom - frame.top,
                                   NULL,
                                   NULL,
                                   hInst,
                                   NULL);
    }

    if (hwndMain == NULL)
    {
        ReportErr(1000);
        return 0;
    }

    SetGraphicsDpi(GetDpiForWindow(hwndMain));
    AdjustWindow(fResize);

    if (SetTimer(hwndMain, ID_TIMER, 1000, NULL) == 0)
    {
        ReportErr(ID_ERR_TIMER);
        DestroyWindow(hwndMain);
        return 0;
    }

    if (!FInitLocal())
    {
        ReportErr(ID_ERR_MEM);
        DestroyWindow(hwndMain);
        return 0;
    }

    fGraphicsInitialized = TRUE;

    SetMenuBar(Preferences.fMenu);
    StartGame();

    ShowWindow(hwndMain, SW_SHOWNORMAL);
    UpdateWindow(hwndMain);
    bInitMinimized = FALSE;

    while ((messageResult = GetMessageA(&msg, NULL, 0, 0)) > 0)
    {
        if (hAccel == NULL || !TranslateAcceleratorA(hwndMain, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    CleanUp();

    if (fUpdateIni)
        WritePreferences();

    return messageResult < 0 ? 0 : (int)msg.wParam;
}
