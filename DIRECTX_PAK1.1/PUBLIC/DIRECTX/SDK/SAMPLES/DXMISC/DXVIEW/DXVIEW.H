/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: dxview.c

    PURPOSE: DirectX Device Viewer Main header file

****************************************************************************/

#include <windows.h>
#ifndef RC_INVOKED
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <tchar.h>

#include <ddraw.h>
#include <dsound.h>
#include <dinput.h>
#endif

#include "caps.h"

//================================================================
// Defines
//================================================================

// Window Width and Height
#define DXView_WIDTH       640
#define DXView_HEIGHT      420

#define DEF_TAB_SIZE        3

// Child controls
#define IDC_LV          0x2000
#define IDC_TV          0x2003

// Imagelist first and last icons
#define IDI_FIRSTIMAGE  IDI_DIRECTX
#define IDI_LASTIMAGE   IDI_DIRECTX_NODE

#define IDI_DIRECTX                     100
#define IDI_CAPS                        101
#define IDI_CAPSOPEN                    102
#define IDI_DIRECTX_NODE                103

#define ID_MENU                         200

#define IDC_SPLIT                       300

#define IDD_ABORTPRINTDLG               1001

#define IDM_EXIT                        40001
#define IDM_ABOUT                       40002
#define IDM_VIEWAVAIL                   40003
#define IDM_VIEWALL                     40004
#define IDM_PRINT                       40005
#define IDM_PRINTALL                    40006
#define IDM_PRINTTOFILE                 40007

#define IDS_PRINT_WARNING               1
#define IDS_PRINT_NEEDSELECT            2
#define IDS_PRINT_NOROOT                3


//================================================================
// Typedefs
//================================================================

typedef struct
{
    HDC         hdcPrint;       // In:      Printer DC
    HWND        hTreeWnd;       // In:      tree window
    HTREEITEM   hCurrTree;      // In:      current tree item handle
    DWORD       dwCharWidth;    // In:      average char width
    DWORD       dwLineHeight;   // In:      max line height
    DWORD       dwCurrLine;     // In/Out:  curr line position on page
    DWORD       dwCharsPerLine; // In:      maximum chars per line (based on avg. char width)
    DWORD       dwLinesPerPage; // In:      maximum lines per page
    DWORD       dwCurrIndent;   // In:      Current tab setting
    BOOL        fStartPage;     // In/Out:  need to a start new page ?!?
} PRINTCBINFO;

typedef void (*SELCALLBACK)(LPARAM lParam1, LPARAM lParam2);
typedef BOOL (*PRINTCALLBACK)(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo);

typedef struct
{
    SELCALLBACK     Callback;
    LPARAM          lParam1;
    LPARAM          lParam2;
    PRINTCALLBACK   printCallback;
} NODEINFO;

typedef struct
{
    TCHAR * szName;         // name of cap
    DWORD   dwOffset;       // offset to cap
    DWORD   dwFlag;         // bit flag for cal
} CAPDEF;

typedef struct
{
    TCHAR * szName;         // name of cap
    SELCALLBACK     Callback;
    LPARAM          lParam2;
    PRINTCALLBACK   printCallback;
} CAPDEFS;

typedef struct
{
    GUID    guidInstance;
    DWORD   dwRefresh;
} LV_INSTANCEGUIDSTRUCT, * LPLV_INSTANCEGUIDSTRUCT;


typedef HRESULT (WINAPI * PFN_DSCREATE)(LPGUID lpguid, LPDIRECTSOUND * ppDS, IUnknown FAR * pUnkOuter);
typedef BOOL (WINAPI * PFN_DSENUMERATE)(LPDSENUMCALLBACK lpDSEnumCallback, LPVOID lpContext);

typedef HRESULT (WINAPI * PFN_DSCCREATE)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC,LPUNKNOWN pUnkOuter);
typedef BOOL (WINAPI * PFN_DSCENUMERATE)(LPDSENUMCALLBACK lpDSEnumCallback,LPVOID lpContext);


//================================================================
// Function prototypes
//================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow);

//================================================================
// DXView treeview/listview helper functions
//================================================================

BOOL DXView_OnCreate(HWND hwnd);
void DXView_OnCommand(HWND hwnd, WPARAM wParam);
void DXView_OnSize(HWND hwnd);
void DXView_OnTreeSelect(HWND hwndTV, NM_TREEVIEW *ptv);
void DXView_OnListViewDblClick(HWND hwndLV, NM_LISTVIEW *plv);
void DXView_Cleanup(void);
void DXView_FillTree(HWND hwndTV);
BOOL DXView_InitImageList(void);
BOOL DXView_OnPrint (HWND hWindow, HWND hTreeView, BOOL fPrintAll);
BOOL DXView_OnFile (HWND hWindow, HWND hTreeWnd,BOOL fPrintAll);

void LVAddColumn(HWND hwndLV, int i, TCHAR *name, int width);
int  LVAddText(HWND hwndLV, int col, const TCHAR *sz, ...);
void LVDeleteAllItems(HWND hwndLV);

HTREEITEM TVAddNode(HTREEITEM hParent, TCHAR *szText,
                    BOOL fKids, int iImage,
                    SELCALLBACK Callback,
                    LPARAM lParam1, LPARAM lParam2,
                    PRINTCALLBACK printCallback);

void AddCapsToTV(HTREEITEM hParent, CAPDEFS *pcds, LPARAM lParam1);
void AddColsToLV(void);
void AddCapsToLV(CAPDEF *pcd, LPVOID pv);
void AddMoreCapsToLV(CAPDEF *pcd, LPVOID pv);
BOOL PrintCapsToDC(CAPDEF *pcd, LPVOID pv, PRINTCBINFO * lpInfo);

// Caps add to ListView callbacks
void DDAddCaps(LPARAM lParam1, LPARAM lParam2);
void DSAddCaps(LPARAM lParam1, LPARAM lParam2);
void DSCAddCaps(LPARAM lParam1, LPARAM lParam2);
void DDAddVideoModes(LPARAM lParam1, LPARAM lParam2);
void DDFourCCFormat(LPARAM lParam1, LPARAM lParam2);

// Caps Print Callbacks
BOOL DDPrintCaps (LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo);
BOOL DSPrintCaps(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo);
BOOL DSCPrintCaps(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo);
BOOL DDPrintVideoModes(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo);
BOOL DDPrintFourCCFormat(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo);

// Printer Helper functions
BOOL PrintStartPage (PRINTCBINFO * pci);
BOOL PrintEndPage (PRINTCBINFO * pci);
BOOL PrintLine (int x, int y, LPCTSTR lpszBuff, DWORD cchBuff, PRINTCBINFO * pci);
BOOL PrintNextLine (PRINTCBINFO * pci);
BOOL PrintValueLine(TCHAR * szText, DWORD dwValue, PRINTCBINFO *lpInfo);

// DirectMusic helper functions
HRESULT DM_FillTree(void);

// Utilities
BOOL Int2Str(LPTSTR pszDest, DWORD i);

HRESULT CALLBACK DSEnumCallBack(const GUID *lpGUID, LPCTSTR lpDriverDesc, LPCTSTR lpDriverName, LPVOID lpContext);
HRESULT CALLBACK DSCEnumCallBack(const GUID *lpGUID, LPCTSTR lpDriverDesc, LPCTSTR lpDriverName, LPVOID lpContext);

#ifndef ZeroMemory
#define ZeroMemory(dst, length)     memset(dst, 0, length)
#endif // !ZeroMemory

//================================================================
// DXView external variables
//================================================================

extern HINSTANCE   g_hInstance;
extern HWND        g_hwndLV;        // List view
extern HWND        g_hwndTV;        // Tree view

extern TCHAR       g_szAppName[];
extern TCHAR       g_szClassName[];
extern TCHAR       g_szTitle[];
extern HWND        g_hwndMain;
extern DWORD       g_dwViewState;
extern DWORD       g_tmAveCharWidth;
extern UINT        g_idTimer;

extern HINSTANCE g_hDSoundLib;
extern BOOL g_PrintToFile;

extern TCHAR c_szYes[];
extern TCHAR c_szNo[];
extern TCHAR c_szCurrentMode[];

