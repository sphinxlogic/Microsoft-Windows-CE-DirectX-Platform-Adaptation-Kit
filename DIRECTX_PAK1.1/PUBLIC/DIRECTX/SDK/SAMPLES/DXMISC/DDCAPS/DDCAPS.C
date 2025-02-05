//=========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
// Copyright (c) 1995 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//=========================================================================

#ifndef STRICT
#define STRICT
#endif

#include <WINDOWS.H>
#include <WINDOWSX.H>
#include <commctrl.h>

#pragma pack(4)

#include "DDCAPS.H"
#include "RESOURCE.H"
#include "DDRAWI.H"
#include "lbprintf.h"

#pragma pack()

//
//  Constants
//
#define MAX_MODES   256         // 256 display modes maximum
#define MAX_BOXES   32
#define MAX_DRIVERS 32          // 32 drivers maximum


//
//  Function prototypes
//
void CheckDlgButton(HWND hwnd, int nIndex, BOOL bState);
BOOL IsDlgButtonChecked(HWND hwnd, int nIndex);
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void ReadDDrawCaps(void);

//
//  Structures
//
struct _modes
{
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD dwBPP;
    DWORD dwRefreshRate;
} Modes[MAX_MODES];

typedef struct capsDef          // hold caps definitions
{
    TCHAR *szName;
    DWORD dwPos;
}
CAPSDEFSTYPE, *LPCAPSDEFSTYPE;

struct _gcaps
{
    LPDWORD caps;
    LPCAPSDEFSTYPE defs;

} gCaps[10];

struct
{
    TCHAR       szDescription[128];
    TCHAR       szName[128];
    GUID        *pGUID;
    GUID        GUIDcopy;
    HMONITOR    hm;
} Drivers[MAX_DRIVERS];


//
//  Global data
//
int Which = 0;
int ModeCnt = 0;                // keeps track of number of modes

HINSTANCE ghInstance;
TCHAR szClassName[] = TEXT("DdrawCaps");
TCHAR szAppName[]   = TEXT("Direct Draw Caps");

int gDriverCnt = 0;             // keeps track of number of drivers
GUID *gpSelectedDriverGUID;

LPDIRECTDRAW lpDD;              // DirectDraw object

// Map caps bits to readable strings
CAPSDEFSTYPE ddCapsDefs[MAX_BOXES] =
{
    {TEXT("3D"), DDCAPS_3D},
    {TEXT("ALIGNBOUNDARYDEST"), DDCAPS_ALIGNBOUNDARYDEST},
    {TEXT("ALIGNSIZEDEST"), DDCAPS_ALIGNSIZEDEST},
    {TEXT("ALIGNBOUNDARYSRC"), DDCAPS_ALIGNBOUNDARYSRC},
    {TEXT("ALIGNSIZESRC"), DDCAPS_ALIGNSIZESRC},
    {TEXT("ALIGNSTRIDE"), DDCAPS_ALIGNSTRIDE},
    {TEXT("BLT"), DDCAPS_BLT},
    {TEXT("BLTQUEUE"), DDCAPS_BLTQUEUE},
    {TEXT("BLTFOURCC"), DDCAPS_BLTFOURCC},
    {TEXT("BLTSTRETCH"), DDCAPS_BLTSTRETCH},
    {TEXT("GDI"), DDCAPS_GDI},
    {TEXT("OVERLAY"), DDCAPS_OVERLAY},
    {TEXT("OVERLAYCANTCLIP"), DDCAPS_OVERLAYCANTCLIP},
    {TEXT("OVERLAYFOURCC"), DDCAPS_OVERLAYFOURCC},
    {TEXT("OVERLAYSTRETCH"), DDCAPS_OVERLAYSTRETCH},
    {TEXT("PALETTE"), DDCAPS_PALETTE},
    {TEXT("PALETTEVSYNC"), DDCAPS_PALETTEVSYNC},
    {TEXT("READSCANLINE"), DDCAPS_READSCANLINE},
    {TEXT("STEREOVIEW"), DDCAPS_STEREOVIEW},
    {TEXT("VBI"), DDCAPS_VBI},
    {TEXT("ZBLTS"), DDCAPS_ZBLTS},
    {TEXT("ZOVERLAYS"), DDCAPS_ZOVERLAYS},
    {TEXT("COLORKEY"), DDCAPS_COLORKEY},
    {TEXT("ALPHA"), DDCAPS_ALPHA},
    {TEXT("CKEYHWASSIST"), DDCAPS_COLORKEYHWASSIST},
    {TEXT("NOHARDWARE"), DDCAPS_NOHARDWARE},
    {TEXT("BLTCOLORFILL"), DDCAPS_BLTCOLORFILL},
    {TEXT("BANKSWITCHED"), DDCAPS_BANKSWITCHED},
    {TEXT("BLTDEPTHFILL"), DDCAPS_BLTDEPTHFILL},
    {TEXT("CANCLIP"), DDCAPS_CANCLIP},
    {TEXT("CANCLIPSTRETCHED"), DDCAPS_CANCLIPSTRETCHED},
};

CAPSDEFSTYPE CKeyCapsDefs[MAX_BOXES] =
{
    {TEXT("DESTBLT"), DDCKEYCAPS_DESTBLT},
    {TEXT("DESTBLTCLRSPACE"), DDCKEYCAPS_DESTBLTCLRSPACE},
    {TEXT("DESTBLTCLRSPACEYUV"), DDCKEYCAPS_DESTBLTCLRSPACEYUV},
    {TEXT("DESTBLTYUV"), DDCKEYCAPS_DESTBLTYUV},
    {TEXT("DESTOVERLAY"), DDCKEYCAPS_DESTOVERLAY},
    {TEXT("DESTOVERLAYCLRSPACE"), DDCKEYCAPS_DESTOVERLAYCLRSPACE},
    {TEXT("DESTOVERLAYCLRSPACEYUV"), DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV},
    {TEXT("DESTOVERLAYONEACTIVE"), DDCKEYCAPS_DESTOVERLAYONEACTIVE},
    {TEXT("DESTOVERLAYYUV"), DDCKEYCAPS_DESTOVERLAYYUV},
    {TEXT("SRCBLT"), DDCKEYCAPS_SRCBLT},
    {TEXT("SRCBLTCLRSPACE"), DDCKEYCAPS_SRCBLTCLRSPACE},
    {TEXT("SRCBLTCLRSPACEYUV"), DDCKEYCAPS_SRCBLTCLRSPACEYUV},
    {TEXT("SRCBLTYUV"), DDCKEYCAPS_SRCBLTYUV},
    {TEXT("SRCOVERLAY"), DDCKEYCAPS_SRCOVERLAY},
    {TEXT("SRCOVERLAYCLRSPACE"), DDCKEYCAPS_SRCOVERLAYCLRSPACE},
    {TEXT("SRCOVERLAYCLRSPACEYUV"), DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV},
    {TEXT("SRCOVERLAYONEACTIVE"), DDCKEYCAPS_SRCOVERLAYONEACTIVE},
    {TEXT("SRCOVERLAYYUV"), DDCKEYCAPS_SRCOVERLAYYUV}
};

CAPSDEFSTYPE FXCapsDefs[MAX_BOXES] =
{
    { TEXT("BLTARITHSTRETCHY"), DDFXCAPS_BLTARITHSTRETCHY },
    { TEXT("BLTARITHSTRETCHYN"), DDFXCAPS_BLTARITHSTRETCHYN },
    { TEXT("BLTMIRRORLEFTRIGHT"), DDFXCAPS_BLTMIRRORLEFTRIGHT },
    { TEXT("BLTMIRRORUPDOWN"), DDFXCAPS_BLTMIRRORUPDOWN },
    { TEXT("BLTROTATION"), DDFXCAPS_BLTROTATION },
    { TEXT("BLTROTATION90"), DDFXCAPS_BLTROTATION90 },
    { TEXT("BLTSHRINKX"), DDFXCAPS_BLTSHRINKX },
    { TEXT("BLTSHRINKXN"), DDFXCAPS_BLTSHRINKXN },
    { TEXT("BLTSHRINKY"), DDFXCAPS_BLTSHRINKY },
    { TEXT("BLTSHRINKYN"), DDFXCAPS_BLTSHRINKYN },
    { TEXT("BLTSTRETCHX"), DDFXCAPS_BLTSTRETCHX },
    { TEXT("BLTSTRETCHXN"), DDFXCAPS_BLTSTRETCHXN },
    { TEXT("BLTSTRETCHY"), DDFXCAPS_BLTSTRETCHY },
    { TEXT("BLTSTRETCHYN"), DDFXCAPS_BLTSTRETCHYN },
    { TEXT("OVERLAYARITHSTRETCHY"), DDFXCAPS_OVERLAYARITHSTRETCHY },
    { TEXT("OVERLAYARITHSTRETCHYN"), DDFXCAPS_OVERLAYARITHSTRETCHYN },
    { TEXT("OVERLAYSHRINKX"), DDFXCAPS_OVERLAYSHRINKX },
    { TEXT("OVERLAYSHRINKXN"), DDFXCAPS_OVERLAYSHRINKXN },
    { TEXT("OVERLAYSHRINKY"), DDFXCAPS_OVERLAYSHRINKY },
    { TEXT("OVERLAYSHRINKYN"), DDFXCAPS_OVERLAYSHRINKYN },
    { TEXT("OVERLAYSTRETCHX"), DDFXCAPS_OVERLAYSTRETCHX },
    { TEXT("OVERLAYSTRETCHXN"), DDFXCAPS_OVERLAYSTRETCHXN },
    { TEXT("OVERLAYSTRETCHY"), DDFXCAPS_OVERLAYSTRETCHY },
    { TEXT("OVERLAYSTRETCHYN"), DDFXCAPS_OVERLAYSTRETCHYN },
    { TEXT("OVERLAYMIRRORLEFTRIGHT"), DDFXCAPS_OVERLAYMIRRORLEFTRIGHT },
    { TEXT("OVERLAYMIRRORUPDOWN"), DDFXCAPS_OVERLAYMIRRORUPDOWN },
};

CAPSDEFSTYPE PalCapsDefs[MAX_BOXES] =
{
    {TEXT("1BIT"), DDPCAPS_1BIT},
    {TEXT("2BIT"), DDPCAPS_2BIT},
    {TEXT("4BIT"), DDPCAPS_4BIT},
    {TEXT("8BITENTRIES"), DDPCAPS_8BITENTRIES},
    {TEXT("8BIT"), DDPCAPS_8BIT},
    {TEXT("ALLOW256"), DDPCAPS_ALLOW256},
    {TEXT("INITIALIZE"), DDPCAPS_INITIALIZE},
    {TEXT("PRIMARYSURFACE"), DDPCAPS_PRIMARYSURFACE},
    {TEXT("PRIMARYSURFACELEFT"), DDPCAPS_PRIMARYSURFACELEFT},
    {TEXT("VSYNC"), DDPCAPS_VSYNC}
};

CAPSDEFSTYPE SurfCapsDefs[MAX_BOXES] =
{
    {TEXT("3DDEVICE"), DDSCAPS_3DDEVICE},
    {TEXT("ALPHA"), DDSCAPS_ALPHA},
    {TEXT("BACKBUFFER"), DDSCAPS_BACKBUFFER},
    {TEXT("COMPLEX"), DDSCAPS_COMPLEX},
    {TEXT("FLIP"), DDSCAPS_FLIP},
    {TEXT("FRONTBUFFER"), DDSCAPS_FRONTBUFFER},
    {TEXT("MIPMAP"), DDSCAPS_MIPMAP},
    {TEXT("OFFSCREENPLAIN"), DDSCAPS_OFFSCREENPLAIN},
    {TEXT("OVERLAY"), DDSCAPS_OVERLAY},
    {TEXT("PALETTE"), DDSCAPS_PALETTE},
    {TEXT("PRIMARYSURFACE"), DDSCAPS_PRIMARYSURFACE},
    {TEXT("PRIMARYSURFACELEFT"), DDSCAPS_PRIMARYSURFACELEFT},
    {TEXT("SYSTEMMEMORY"), DDSCAPS_SYSTEMMEMORY},
    {TEXT("TEXTURE"), DDSCAPS_TEXTURE},
    {TEXT("VIDEOMEMORY"), DDSCAPS_VIDEOMEMORY},
    {TEXT("VISIBLE"), DDSCAPS_VISIBLE},
    {TEXT("WRITEONLY"), DDSCAPS_WRITEONLY},
    {TEXT("ZBUFFER"), DDSCAPS_ZBUFFER}
};



//*------------------------------------------------------------------------
//| Dlg_OnCommand
//*------------------------------------------------------------------------
BOOL
Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDCANCEL:
        case IDC_EXIT:
            DestroyWindow(hwnd);
            break;
    }

    return TRUE;
}

//*------------------------------------------------------------------------
//| IsDlgButtonChecked
//*------------------------------------------------------------------------
BOOL IsDlgButtonChecked(HWND hwnd, int nIndex)
{
    return (SendDlgItemMessage(hwnd, nIndex, BM_GETCHECK, 0, 0));
}

//*------------------------------------------------------------------------
//| CheckDlgButton
//*------------------------------------------------------------------------
void CheckDlgButton(HWND hwnd, int nIndex, BOOL bState)
{
    SendDlgItemMessage(hwnd, nIndex, BM_SETCHECK, bState, 0);
}

//*------------------------------------------------------------------------
//| Dlg_OnInitDialog
//*------------------------------------------------------------------------
BOOL
Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    int i;
    TCHAR szTitle[256];
    static TCHAR *rgszCaps[] = {
        TEXT("Direct Draw Caps Bits"),
        TEXT("Direct Draw Color Key Caps Bits"),
        TEXT("Direct Draw FX Caps Bits"),
        TEXT("Direct Draw Palette Caps Bits"),
        TEXT("Direct Draw Surface Caps Bits")};

    wsprintf(szTitle, TEXT("%s (%s)"), rgszCaps[Which%5],
            (Which < 5) ? TEXT("HAL") : TEXT("HEL"));
    SetWindowText(hwnd, szTitle);

    ShowWindow(GetDlgItem(hwnd, IDC_RESTORE), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_REFRESH), SW_HIDE);

    for (i = 0; i < MAX_BOXES; i++)
    {
        if (gCaps[Which].defs[i].szName == NULL)
        {
            SetDlgItemText(hwnd, i + IDC_CHECK1, TEXT(""));
            ShowWindow( GetDlgItem(hwnd, i + IDC_CHECK1), SW_HIDE );
        }
        else
        {
            SetDlgItemText(hwnd, i + IDC_CHECK1, gCaps[Which].defs[i].szName);
            ShowWindow( GetDlgItem(hwnd, i + IDC_CHECK1), SW_SHOW );
        }
        CheckDlgButton(hwnd, i + IDC_CHECK1,
                       (gCaps[Which].defs[i].dwPos & *gCaps[Which].caps) != 0);
    }
    return TRUE;
}

//*------------------------------------------------------------------------
//| ddCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
ddCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 0;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}

//*------------------------------------------------------------------------
//| CKeyCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
CKeyCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 1;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}

//*------------------------------------------------------------------------
//| SurfCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
SurfCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 4;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}

//*------------------------------------------------------------------------
//| FXCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
FXCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 2;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}


//*------------------------------------------------------------------------
//| PalCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
PalCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 3;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}


//*------------------------------------------------------------------------
//| HELddCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
HELddCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 5;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}

//*------------------------------------------------------------------------
//| HELCKeyCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
HELCKeyCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 6;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}

//*------------------------------------------------------------------------
//| HELSurfCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
HELSurfCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 9;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}

//*------------------------------------------------------------------------
//| HELFXCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
HELFXCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 7;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}


//*------------------------------------------------------------------------
//| HELPalCapsDlg_Proc
//*------------------------------------------------------------------------
BOOL CALLBACK
HELPalCapsDlg_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = TRUE;

    Which = 8;
    switch (uMsg)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, Dlg_OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, Dlg_OnCommand);

        default:
            fProcessed = FALSE;
            break;
    }
    return (fProcessed);
}


//*------------------------------------------------------------------------
//| DDEnumCallback
//*------------------------------------------------------------------------
BOOL WINAPI
DDEnumCallbackEx(GUID *pGUID, LPTSTR pDescription, LPTSTR pName, LPVOID pContext, HMONITOR hm)
{
    if (pGUID)
    {
        Drivers[gDriverCnt].GUIDcopy = *pGUID;
        Drivers[gDriverCnt].pGUID = &Drivers[gDriverCnt].GUIDcopy;
    }
    else
        Drivers[gDriverCnt].pGUID = NULL;

    Drivers[gDriverCnt].szDescription[127] = TEXT('\0');
    Drivers[gDriverCnt].szName[127] = TEXT('\0');
    _tcsncpy(Drivers[gDriverCnt].szDescription,pDescription,127);
    _tcsncpy(Drivers[gDriverCnt].szName,pName,127);
    Drivers[gDriverCnt].hm = hm;

    if (gDriverCnt < MAX_DRIVERS)
        gDriverCnt++;
    else
        return DDENUMRET_CANCEL;

    return DDENUMRET_OK;
}

BOOL WINAPI
DDEnumCallback(GUID *pGUID, LPTSTR pDescription, LPTSTR pName, LPVOID context)
{
    return (DDEnumCallbackEx(pGUID, pDescription, pName, context, NULL));
}


//*------------------------------------------------------------------------
//| EnumModesCallback
//*------------------------------------------------------------------------
static int  gCurMode = 0;

DWORD DDAPI
EnumModesCallback(LPDDSURFACEDESC pddsd, LPVOID context)
{
    Modes[ModeCnt].dwWidth = pddsd->dwWidth;
    Modes[ModeCnt].dwHeight = pddsd->dwHeight;
    Modes[ModeCnt].dwBPP = pddsd->ddpfPixelFormat.dwRGBBitCount;
    Modes[ModeCnt].dwRefreshRate = pddsd->dwRefreshRate;

    LBPrintf(TEXT("Mode %2d: %4d x %4d  %2d bpp %d Hz"), ModeCnt, Modes[ModeCnt].dwWidth,
             Modes[ModeCnt].dwHeight, Modes[ModeCnt].dwBPP, Modes[ModeCnt].dwRefreshRate);

    if (ModeCnt < MAX_MODES)
        ModeCnt++;
    else
        return DDENUMRET_CANCEL;

    return DDENUMRET_OK;
}


//*------------------------------------------------------------------------
//| WinMain:
//|     Parameters:
//|         hInstance     - Handle to current Data Segment
//|         hPrevInstance - Always NULL in Win32
//|         lpszCmdLine   - Pointer to command line info
//|         nCmdShow      - Integer value specifying how to start app.,
//|                            (Iconic [7] or Normal [1,5])
//*------------------------------------------------------------------------
int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPWSTR lpszCmdLine,
        int nCmdShow)
{
    int                     nReturn = 0;
    HRESULT                 ddrval;
    HINSTANCE               hDDrawDLL;
    LPDIRECTDRAWENUMERATEEX pDirectDrawEnumerateEx;

    ghInstance = hInstance;

    // Pop up a dialog letting the user select a driver to use (if more than one)
    gpSelectedDriverGUID = NULL;

    // You need to do a GetModuleHandle and GetProcAddress in order to get the
    // DirectDrawEnumerateEx
    hDDrawDLL = GetModuleHandle(TEXT("DDRAW"));
    if (!hDDrawDLL)
    {
        OutputDebugString(TEXT("LoadLibrary() failed - ddcaps: Can't list display modes"));
        return (-1);
    }
    pDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEX )
        GetProcAddress(hDDrawDLL,TEXT("DirectDrawEnumerateExW"));

    if (pDirectDrawEnumerateEx)
        pDirectDrawEnumerateEx( DDEnumCallbackEx, (LPVOID) NULL,
                                DDENUM_ATTACHEDSECONDARYDEVICES |
                                DDENUM_DETACHEDSECONDARYDEVICES |
                                DDENUM_NONDISPLAYDEVICES);
    else    // Old DirectDraw, so do it the old way
        DirectDrawEnumerate(DDEnumCallback, NULL);

    ddrval = DirectDrawCreate(gpSelectedDriverGUID, &lpDD, NULL);
    if (ddrval != DD_OK)
    {
        OutputDebugString(TEXT("DirectDrawCreate failed"));
        return (-1);
    }

    // Read DirectDraw capabilities
    ReadDDrawCaps();

    if (Init(hInstance, hPrevInstance, lpszCmdLine, nCmdShow))
    {
        nReturn = DoMain(hInstance);
        CleanUp();
    }

    return nReturn;
}


//*------------------------------------------------------------------------
//| ReadDDrawCaps
//*------------------------------------------------------------------------
void ReadDDrawCaps(void)
{
    static DDCAPS ddcapsHEL, ddcapsHAL;

    ddcapsHEL.dwSize = sizeof(ddcapsHEL);
    ddcapsHAL.dwSize = sizeof(ddcapsHAL);

    lpDD->lpVtbl->GetCaps(lpDD, &ddcapsHAL, &ddcapsHEL);

    gCaps[0].caps = &(ddcapsHAL.dwCaps);
    gCaps[0].defs = ddCapsDefs;

    gCaps[1].caps = &(ddcapsHAL.dwCKeyCaps);
    gCaps[1].defs = CKeyCapsDefs;

    gCaps[2].caps = &(ddcapsHAL.dwFXCaps);
    gCaps[2].defs = FXCapsDefs;

    gCaps[3].caps = &(ddcapsHAL.dwPalCaps);
    gCaps[3].defs = PalCapsDefs;

    gCaps[4].caps = &(ddcapsHAL.ddsCaps.dwCaps);
    gCaps[4].defs = SurfCapsDefs;

    gCaps[5].caps = &(ddcapsHEL.dwCaps);
    gCaps[5].defs = ddCapsDefs;

    gCaps[6].caps = &(ddcapsHEL.dwCKeyCaps);
    gCaps[6].defs = CKeyCapsDefs;

    gCaps[7].caps = &(ddcapsHEL.dwFXCaps);
    gCaps[7].defs = FXCapsDefs;

    gCaps[8].caps = &(ddcapsHEL.dwPalCaps);
    gCaps[8].defs = PalCapsDefs;

    gCaps[9].caps = &(ddcapsHEL.ddsCaps.dwCaps);
    gCaps[9].defs = SurfCapsDefs;
}


//*------------------------------------------------------------------------
//| Init
//|     Initialization for the program is done here:
//|
//*------------------------------------------------------------------------
BOOL
Init(HINSTANCE hInstance, HINSTANCE hPrevInstance,
     LPTSTR lpszCmdLine, int nCmdShow)
{
    WNDCLASS rClass;
    HWND hWnd1;
    HWND hwndLB;
    HWND hwndCmdBar;
    RECT rc = { 0 };

    rClass.style = 0;
    rClass.lpfnWndProc = (WNDPROC) OverlappedWinProc;
    rClass.cbClsExtra = 0;
    rClass.cbWndExtra = 0;
    rClass.hInstance = hInstance;
    rClass.hIcon = LoadIcon(hInstance, TEXT("DDCAPS_ICON"));
    rClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    rClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    rClass.lpszMenuName = NULL;
    rClass.lpszClassName = szClassName;

    if (!RegisterClass(&rClass))
    {
        return FALSE;
    }

    hWnd1 = CreateWindowEx(0,
                           szClassName,
                           szAppName,
                           WS_OVERLAPPED | WS_VISIBLE | WS_DLGFRAME | WS_CAPTION,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           300,
                           200,
                           NULL,
                           NULL,
                           hInstance,
                           NULL);

    if (!hWnd1)
    {
        return FALSE;
    }

    hwndCmdBar = CommandBar_Create(hInstance, hWnd1, 1);
    if (!hwndCmdBar)
    {
        return FALSE;
    }

    CommandBar_InsertMenubar(hwndCmdBar, hInstance, IDR_MENU1, 0);
    CommandBar_AddAdornments(hwndCmdBar, 0, 0);
    CommandBar_DrawMenuBar(hwndCmdBar, 0);

    GetWindowRect(hwndCmdBar, &rc);

    hwndLB = LBCreate(hWnd1, hInstance, rc.bottom-rc.top, FALSE);

    ShowWindow(hWnd1, nCmdShow);
    UpdateWindow(hWnd1);

    lpDD->lpVtbl->EnumDisplayModes(lpDD, 0, NULL, (LPVOID) hwndLB, EnumModesCallback);

    // highlight the current mode
    SendMessage(hwndLB, LB_SETCURSEL, gCurMode, 0L);

    return (int) hWnd1;
}

//*------------------------------------------------------------------------
//| DoMain:
//|     This is the main loop for the application:
//*------------------------------------------------------------------------
int
DoMain(HINSTANCE hInstance)
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

//*------------------------------------------------------------------------
//| CleanUp:
//|     Any last-minute application cleanup activities are done here:
//*------------------------------------------------------------------------
void
CleanUp(void)
{
}

//*------------------------------------------------------------------------
//| OverlappedWinProc:
//|
//*------------------------------------------------------------------------
LRESULT WINAPI
OverlappedWinProc(HWND hWnd,
                   UINT wMsgID,
                   UINT wParam,
                   LPARAM lParam)
{
    UINT sel;
    HRESULT ddrval;

    switch (wMsgID)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_FILE_EXIT:
                    SendMessage(hWnd, WM_CLOSE, 0, 0L);
                    break;

                case IDD_ABOUT:
                    DialogBox(ghInstance, TEXT("About"), hWnd, (DLGPROC)AboutDlgProc);
                    break;

                case ID_VIEW_DRIVERCAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, ddCapsDlg_Proc);
                    break;

                case ID_VIEW_DRIVERCOLORKEYCAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, CKeyCapsDlg_Proc);
                    break;

                case ID_VIEW_DRIVERFXCAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, FXCapsDlg_Proc);
                    break;

                case ID_VIEW_DRIVERPALETTECAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, PalCapsDlg_Proc);
                    break;

                case ID_VIEW_DRIVERSURFACECAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, SurfCapsDlg_Proc);
                    break;

                case ID_VIEW_HELCAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, HELddCapsDlg_Proc);
                    break;

                case ID_VIEW_HELCOLORKEYCAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, HELCKeyCapsDlg_Proc);
                    break;

                case ID_VIEW_HELFXCAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, HELFXCapsDlg_Proc);
                    break;

                case ID_VIEW_HELPALETTECAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, HELPalCapsDlg_Proc);
                    break;

                case ID_VIEW_HELSURFACECAPS:
                    CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1),
                                 NULL, HELSurfCapsDlg_Proc);
                    break;

                case ID_PRINTF_LISTBOX:
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        sel = (UINT) SendMessage((HWND) lParam, LB_GETCURSEL, 0,
                                                 0L);
                        if (sel < (UINT)ModeCnt)
                        {
                            ddrval = lpDD->lpVtbl->SetCooperativeLevel(lpDD,
                                                        hWnd,
                                                        DDSCL_EXCLUSIVE |
                                                        DDSCL_NOWINDOWCHANGES |
                                                        DDSCL_FULLSCREEN);
                            if (ddrval == DD_OK)
                            {
                                ddrval = lpDD->lpVtbl->SetDisplayMode(lpDD,
                                                        Modes[sel].dwWidth,
                                                        Modes[sel].dwHeight,
                                                        Modes[sel].dwBPP);
                                ddrval = lpDD->lpVtbl->SetCooperativeLevel(lpDD,
                                                        hWnd, DDSCL_NORMAL);
                            }
                        }
                    }
                    else if (HIWORD(wParam) == LBN_SETFOCUS)
                    {
                        SendMessage((HWND) lParam, LB_SETCURSEL, gCurMode, 0L);
                    }
                    break;
            }
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;

                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_SIZE:
            LBSize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, wMsgID, wParam, lParam);
    }
    return 0;
}


//================================================================
//  About - process about box
//================================================================
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            return(TRUE);

        case WM_COMMAND:                      // message: received a command
            if(LOWORD(wParam) == IDOK         // "OK" box selected?
            || LOWORD(wParam) == IDCANCEL) {  // System menu close command?
                EndDialog(hDlg, TRUE);        // Exit the dialog
                return(TRUE);
            }
            break;
    }
    return(FALSE); // Didn't process the message
}

