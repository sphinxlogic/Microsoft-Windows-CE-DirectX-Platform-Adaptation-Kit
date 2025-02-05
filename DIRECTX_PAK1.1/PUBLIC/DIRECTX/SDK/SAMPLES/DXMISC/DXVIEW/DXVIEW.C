/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: dxview.c

    PURPOSE: DirectX Device Viewer

****************************************************************************/

#define INITGUID

#include <stdio.h>
#include "dxview.h"
#include "support.h"
#include "ddinfo.h"

#ifdef SUPPORT_DIRECTSOUND
#include "dsinfo.h"
#pragma comment(lib, "dsound")
#endif

#pragma comment(lib, "dxguid")
#pragma comment(lib, "comctl32")

//
//  Function prototypes
//
void FinishWindowInit();

//
//  Global data
//
TCHAR       g_szAppName[]   = TEXT("DXView");
TCHAR       g_szClassName[] = TEXT("DXViewer");
TCHAR       g_szTitle[]     = TEXT("DirectX Device Viewer\0");

HINSTANCE   g_hInstance=0;
HWND        g_hwndMain=0;
HWND        g_hwndTV=0;        // Tree view
HWND        ghwndCmdBar=0;
HIMAGELIST  g_hImageList=0;
HFONT       g_hFont=0;
HMENU       ghwndMenu=0;

int         g_xPaneSplit=0;
int         g_xHalfSplitWidth=0;
BOOL        g_bSplitMove=0;
DWORD       g_dwViewState=0;
DWORD       g_tmAveCharWidth=0;
int         g_nCmdBarHeight=0;
UINT        g_idTimer=0;

TCHAR c_szYes[] = TEXT("Yes");
TCHAR c_szNo[]  = TEXT("No");
TCHAR c_szCurrentMode[] = TEXT("Current Mode");

IDirectDraw *lpDD;                  // DirectDraw object
IDirectDraw4 *lpDD4;                // DirectDraw object
GUID *       ddid;



//================================================================
//  WinMain - entry point
//================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    g_hInstance = hInstance; // Store instance handle in our global variable

    // Try to load the DirectSound DLL
    g_hDSoundLib = LoadLibrary(TEXT("DSOUND.DLL"));
    if (g_hDSoundLib == NULL)
    {       // couldn't load the library so put up a message and quit.
        MessageBox(g_hwndMain, TEXT("Could not load DSOUND.DLL"), g_szAppName, MB_OK);
        CoUninitialize();
        return 0;       // terminate after showing the message.
    }

    if (InitInstance(hInstance, lpCmdLine, nCmdShow))
    {
        while(GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Unload dsound.dll
    FreeLibrary(g_hDSoundLib);

    CoUninitialize();
    return(msg.wParam);
}


//================================================================
//  InitInstance - create main window
//================================================================
BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS  wc={0};

    wc.style         = 0;                       // Class style(s).
    wc.lpfnWndProc   = (WNDPROC)WndProc;        // Window Procedure
    wc.cbClsExtra    = 0;                       // No per-class extra data.
    wc.cbWndExtra    = 0;                       // No per-window extra data.
    wc.hInstance     = hInstance;               // Owner of this class
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECTX));
    wc.hCursor       = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SPLIT));
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // Default color
    wc.lpszMenuName  = NULL;                    // Must use CE commad bar
    wc.lpszClassName = g_szClassName;           // Name to register

    if(!RegisterClass(&wc))
    {
        return FALSE;
    }

    // Create a main window for this application instance.
    g_hwndMain = CreateWindow(
                    g_szClassName,   // See RegisterClass() call.
                    g_szTitle,       // Text for window title bar.
                    WS_OVERLAPPED | WS_CLIPCHILDREN | WS_DLGFRAME,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,   // Or use DXView_Width
                    CW_USEDEFAULT,   // Or use DXView_Height
                    NULL,            // Overlapped windows have no parent.
                    NULL,            // Use the window class menu.
                    hInstance,       // This instance owns this window.
                    NULL);

    // If window could not be created, return "failure"
    if (! g_hwndMain)
    {
        return(FALSE);
    }

    // Create command bar (replaces menu bar on WinCE)
    ghwndCmdBar = CommandBar_Create(hInstance, g_hwndMain, 1);
    if (ghwndCmdBar)
    {
        CommandBar_InsertMenubar(ghwndCmdBar, hInstance, ID_MENU, 0);
        CommandBar_AddAdornments(ghwndCmdBar, 0, 0);
        CommandBar_DrawMenuBar(ghwndCmdBar, 0);
        CommandBar_Show(ghwndCmdBar, TRUE);

        // Need main menu handle for later updates
        ghwndMenu = CommandBar_GetMenu(ghwndCmdBar, 0);
        if (ghwndMenu)
            CheckMenuItem(ghwndMenu, g_dwViewState, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        RETAILMSG(1, (TEXT("Failed to create command bar!\r\n")));
    }

    // Must create/initialize command bar before creating other child windows
    FinishWindowInit();

    // Make the window visible; update its client area; and return "success"
    ShowWindow (g_hwndMain, nCmdShow); // Show the window

    return(TRUE);              // We succeeded...
}


void FinishWindowInit()
{
    g_nCmdBarHeight = CommandBar_Height(ghwndCmdBar);

    // Create the list view window.
    g_hwndLV = CreateWindowEx(WS_EX_CLIENTEDGE,
                              WC_LISTVIEW,
                              TEXT(""),
                              WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT,
                              0,
                              g_nCmdBarHeight,
                              0, 0,
                              g_hwndMain,
                              (HMENU)IDC_LV,
                              g_hInstance,
                              NULL);

    // Create the tree view window.
    g_hwndTV = CreateWindowEx(WS_EX_CLIENTEDGE,
                              WC_TREEVIEW,
                              TEXT(""),
                              WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES |
                                TVS_HASBUTTONS | TVS_LINESATROOT,
                              0,
                              g_nCmdBarHeight,
                              0, 0,
                              g_hwndMain,
                              (HMENU)IDC_TV,
                              g_hInstance,
                              NULL);

    // Create the image list.
    DXView_InitImageList();

    // Initialize the tree view
    DXView_FillTree(g_hwndTV);
}


//================================================================
//  WndProc - main window proc
//================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CREATE:
            return DXView_OnCreate(hwnd);

        case WM_SIZE:
            DXView_OnSize(hwnd);
            break;

        case WM_LBUTTONDOWN:
            g_bSplitMove = TRUE;
            SetCapture(hwnd);
            break;

        case WM_LBUTTONUP:
            g_bSplitMove = FALSE;
            ReleaseCapture();
            break;

        case WM_MOUSEMOVE:
            if(g_bSplitMove)
            {
                RECT    rect;
                // change the value from unsigned to signed
                int     x = (int)(short)LOWORD(lParam);

                GetClientRect(hwnd, &rect);
                if (rect.left > x)
                {
                    x = rect.left;
                }
                else if (rect.right < x)
                {
                    x = rect.right;
                }
                g_xPaneSplit = (x - g_xHalfSplitWidth);
                DXView_OnSize(hwnd);
            }
            break;

        case WM_NOTIFY:
            if (((NMHDR*)lParam)->hwndFrom == g_hwndTV)
            {
                if (((NMHDR*)lParam)->code == TVN_SELCHANGED)
                    DXView_OnTreeSelect(g_hwndTV, (NM_TREEVIEW*)lParam);
            }

            if (((NMHDR*)lParam)->hwndFrom == g_hwndLV)
            {
                if (((NMHDR*)lParam)->code == NM_RDBLCLK)
                    DXView_OnListViewDblClick(g_hwndLV, (NM_LISTVIEW*)lParam);
            }

            break;

       case WM_COMMAND:  // message: command from application menu
            DXView_OnCommand(hwnd, wParam);
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:  // message: window being destroyed
            DXView_Cleanup();  // Free per item struct for all items
            PostQuitMessage(0);
            break;
    }

    return(DefWindowProc(hwnd, message, wParam, lParam));
}


//================================================================
//================================================================
BOOL DXView_OnCreate(HWND hwnd)
{
    HDC hDC;
    int PixelsPerInch;
    TEXTMETRIC tm;
    static TCHAR szBuf[MAX_PATH];

    hDC = GetDC(hwnd);
    PixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
    g_hFont = GetStockObject(SYSTEM_FONT);
    SelectObject(hDC, g_hFont);
    GetTextMetrics(hDC, &tm);
    g_tmAveCharWidth = tm.tmAveCharWidth;
    ReleaseDC(hwnd, hDC);

    // Initialize global data
    g_dwViewState = IDM_VIEWALL;
    g_xPaneSplit = PixelsPerInch * 12 / 4;
    g_xHalfSplitWidth = GetSystemMetrics(SM_CXBORDER) / 2;

    // Make sure that the common control library read to rock
    InitCommonControls();

    return(TRUE);
}


//================================================================
//================================================================
void AddCapsToTV(HTREEITEM hRoot, CAPDEFS *pcds, LPARAM lParam1)
{
    HTREEITEM hTree;
    HTREEITEM hParent[20];
    TCHAR *name;
    int level=0;
    BOOL fRoot = TRUE; // the first one is always a root

    hParent[0] = hRoot;

    while (name = pcds->szName)
    {
        if (*name == TEXT('-'))
        {
            level--;
            name++;
        }

        if (*name == TEXT('+'))
        {
            fRoot = TRUE;
            name++;
        }

        if (*name)
        {
            hTree = TVAddNode(hParent[level], name, fRoot, IDI_CAPS,
                              pcds->Callback, lParam1,
                              pcds->lParam2, pcds->printCallback);

            if (fRoot)
            {
                level++;
                hParent[level] = hTree;
                fRoot = FALSE;
            }
        }

        pcds++;  // Get next Cap bit definition
    }
}


//================================================================
//================================================================
BOOL PrintCapsToDC(CAPDEF *pcd, LPVOID pv, PRINTCBINFO * lpInfo)
{
    DWORD dwValue;
    DWORD cchLen;
    int   xName, xVal, yLine;

    // Check Parameters
    if ((! pcd) || (!lpInfo))
        return FALSE;

    // Calculate Name and Value column x offsets
    xName   = (lpInfo->dwCurrIndent * DEF_TAB_SIZE * lpInfo->dwCharWidth);
    xVal    = xName + (32 * lpInfo->dwCharWidth);

    while (pcd->szName && *pcd->szName)
    {
        dwValue = *(DWORD *)(((BYTE *)pv) + pcd->dwOffset);
        yLine = lpInfo->dwCurrLine * lpInfo->dwLineHeight;

        if (pcd->dwFlag)
        {
            if (pcd->dwFlag & dwValue)
            {
                // Print Name
                cchLen = _tcslen (pcd->szName);
                if (! PrintLine (xName, yLine, pcd->szName, cchLen, lpInfo))
                    return FALSE;

                // Print Yes in value column
                cchLen = _tcslen (c_szYes);
                if (! PrintLine (xVal, yLine, c_szYes, cchLen, lpInfo))
                    return FALSE;

                // Advance to next line on page
                if (! PrintNextLine (lpInfo))
                    return FALSE;
            }
            else if (g_dwViewState == IDM_VIEWALL)
            {
                // Print name
                cchLen = _tcslen (pcd->szName);
                if (! PrintLine (xName, yLine, pcd->szName, cchLen, lpInfo))
                    return FALSE;

                // Print No in value column
                cchLen = _tcslen (c_szNo);
                if (! PrintLine (xVal, yLine, c_szNo, cchLen, lpInfo))
                    return FALSE;

                // Advance to next line on page
                if (! PrintNextLine (lpInfo))
                    return FALSE;
            }
        }
        else
        {
            TCHAR    szBuff[80];

            // Print name
            wsprintf (szBuff, pcd->szName, TEXT("test"));
            cchLen = _tcslen (pcd->szName);
            if (! PrintLine (xName, yLine, szBuff, cchLen, lpInfo))
                return FALSE;

            // Print value
            Int2Str(szBuff,dwValue);
            cchLen = _tcslen (szBuff);
            if (! PrintLine (xVal, yLine, szBuff, cchLen, lpInfo))
                return FALSE;

            // Advance to next line on page
            if (! PrintNextLine (lpInfo))
                return FALSE;
        }

        pcd++;  // Get next Cap bit definition
    }

    // Success
    return TRUE;
}


//================================================================
//================================================================
void DXView_OnTreeSelect(HWND hwndTV, NM_TREEVIEW *ptv)
{
    NODEINFO *pni;

    SendMessage(g_hwndLV, WM_SETREDRAW, FALSE, 0);
    LVDeleteAllItems(g_hwndLV);
    LVAddColumn(g_hwndLV, 0, TEXT(""), 0);

    if (ptv == NULL)
    {
        TV_ITEM tvi;
        // get lParam of current tree node
        tvi.hItem  = TreeView_GetSelection(g_hwndTV);
        tvi.mask   = TVIF_PARAM;
        tvi.lParam = 0;
        TreeView_GetItem(g_hwndTV, &tvi);
        pni = (NODEINFO*)tvi.lParam;
    }
    else
    {
        pni = (NODEINFO*)ptv->itemNew.lParam;
    }

    if (pni && pni->Callback)
    {
        pni->Callback(pni->lParam1, pni->lParam2);
    }

    SendMessage(g_hwndLV, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(g_hwndLV, NULL, TRUE);
}


//================================================================
//================================================================
void DXView_OnListViewDblClick(HWND hwndLV, NM_LISTVIEW *plv)
{
    LV_ITEM lvi;

    lvi.mask   = LVIF_PARAM;
    lvi.lParam = 0;
    lvi.iItem  = plv->iItem;
    ListView_GetItem(hwndLV, &lvi);
}


//================================================================
//================================================================
void DXView_OnCommand(HWND hwnd, WPARAM wParam)
{
    BOOL  fPrintAll;

    switch(LOWORD(wParam))
    {
        case IDM_VIEWAVAIL:
        case IDM_VIEWALL:
            CheckMenuItem(ghwndMenu, g_dwViewState, MF_BYCOMMAND | MF_UNCHECKED);
            g_dwViewState = LOWORD(wParam);
            CheckMenuItem(ghwndMenu, g_dwViewState, MF_BYCOMMAND | MF_CHECKED);
            DXView_OnTreeSelect(g_hwndTV, NULL);
            break;

        case IDM_PRINT:
        case IDM_PRINTALL:
            // Print all statistics or just current selection
            if (LOWORD(wParam) == IDM_PRINTALL)
                fPrintAll = TRUE;
            else
                fPrintAll = FALSE;
            // Print out statistics
            DXView_OnPrint (hwnd, g_hwndTV, fPrintAll);
            break;

        case IDM_PRINTTOFILE:
            fPrintAll = TRUE;
            g_PrintToFile = TRUE;
            // Print out statistics
            DXView_OnFile (hwnd, g_hwndTV, fPrintAll);
            break;

        case IDM_ABOUT:
            DialogBox(g_hInstance, TEXT("About"), hwnd, (DLGPROC)About);
            break;

        case IDM_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
    }
}


//================================================================
//================================================================
void DXView_Cleanup()
{
    if (lpDD)
        IDirectDraw_Release(lpDD);

    if (lpDS)
        IDirectSound_Release(lpDS);

    if (lpDSC)
        IDirectSoundCapture_Release(lpDSC);

    if(g_hImageList)
        ImageList_Destroy(g_hImageList);
}


//================================================================
//================================================================
BOOL DXView_InitImageList()
{
    int cxSmIcon;
    int cySmIcon;
    UINT Index;
    HICON hIcon;

    if (g_hImageList)
        return TRUE;

    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    cySmIcon = GetSystemMetrics(SM_CYSMICON);

    //  First, create the image list that is needed.
    if((g_hImageList = ImageList_Create(cxSmIcon, cySmIcon,
                            TRUE, IDI_LASTIMAGE - IDI_FIRSTIMAGE, 10)) == NULL)
        return(FALSE);

    //
    //  Initialize the image list with all of the icons that we'll be using
    //  Once set, send its handle to all interested child windows.
    //
    for (Index = IDI_FIRSTIMAGE; Index <= IDI_LASTIMAGE; Index++)
    {
        hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(Index), IMAGE_ICON,
                          cxSmIcon, cySmIcon, 0);
        ImageList_AddIcon(g_hImageList, hIcon);
        DestroyIcon(hIcon);
    }

    TreeView_SetImageList(g_hwndTV, g_hImageList, TVSIL_NORMAL);

    return(TRUE);
}


//================================================================
//================================================================
void DXView_FillTree(HWND hwndTV)
{
    HTREEITEM   hTree;

    PFN_DSENUMERATE pfn_DSEnumerate;
    PFN_DSCENUMERATE pfn_DSCEnumerate;

    // Add direct draw devices

    hTree = TVAddNode(TVI_ROOT, TEXT("DirectDraw Devices"), TRUE,
                      IDI_DIRECTX_NODE, NULL, 0, 0, NULL);

    // Add Display Driver node(s) and capability nodes to treeview
    {
        HINSTANCE h = LoadLibrary(TEXT("ddraw.dll"));
        if (!h)
        {
           MessageBox(NULL, TEXT("LoadLibrary() failed"),
                      TEXT("dxview: Can't list display modes"), MB_OK);
           return ;
        }
        {
            LPDIRECTDRAWENUMERATEEX lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX)
                        GetProcAddress(h,TEXT("DirectDrawEnumerateExW"));
            if (lpDDEnumEx)
               lpDDEnumEx((LPDDENUMCALLBACKEX)DDEnumCallBack2, hTree,
                   DDENUM_ATTACHEDSECONDARYDEVICES |
                   DDENUM_DETACHEDSECONDARYDEVICES |
                   DDENUM_NONDISPLAYDEVICES);
            else
            {   //Do it the old way
               DirectDrawEnumerate(DDEnumCallBack, hTree);
            }
        }
        FreeLibrary(h);
    }

    // Add Hardware Emulation Layer node to treeview
    DDEnumCallBack((GUID *)DDCREATE_EMULATIONONLY,
                   TEXT("Hardware Emulation Layer"), NULL, (LPVOID)hTree);

    TreeView_Expand(g_hwndTV, hTree, TVE_EXPAND);
    TreeView_SelectItem(g_hwndTV, hTree);

    pfn_DSEnumerate = (PFN_DSENUMERATE)GetProcAddress(g_hDSoundLib,
                                        TEXT("DirectSoundEnumerateW"));
    if (pfn_DSEnumerate == NULL)
    {
        MessageBox(g_hwndMain, TEXT("DSOUND.DLL does not implement DirectSoundEnumerate()"),
                   g_szAppName, MB_OK);
    }
    else
    {
        // Add direct sound devices
        hTree = TVAddNode(TVI_ROOT, TEXT("DirectSound Devices"), TRUE,
                                        IDI_DIRECTX_NODE, NULL, 0, 0, NULL);

        (*pfn_DSEnumerate)(DSEnumCallBack, hTree);

        TreeView_Expand(g_hwndTV, hTree, TVE_EXPAND);
    }


    // Get a pointer to the DirectSoundCaptureEnumerate
    pfn_DSCEnumerate = (PFN_DSCENUMERATE)GetProcAddress(g_hDSoundLib,
                                        TEXT("DirectSoundCaptureEnumerateW"));
    if (pfn_DSCEnumerate == NULL)
    {
        MessageBox(g_hwndMain,
                   TEXT("DSOUND.DLL does not implement DirectSoundCaptureEnumerate().")
                   TEXT(" The system may not have DirectX 5.0 installed."), g_szAppName, MB_OK);
    }
    else
    {
        // Add direct sound capture devices
        hTree = TVAddNode(TVI_ROOT, TEXT("DirectSoundCapture Devices"), TRUE,
                          IDI_DIRECTX_NODE, NULL, 0, 0, NULL);
        (*pfn_DSCEnumerate)(DSCEnumCallBack, hTree);
        TreeView_Expand(g_hwndTV, hTree, TVE_EXPAND);
    }
}


//================================================================
//================================================================
// Get number as a string
BOOL Int2Str(LPTSTR pszDest, DWORD i)
{
    TCHAR   szIn[32];
    TCHAR   szOut[32];
    TCHAR   szDec[2];
    TCHAR   *pszDec;

    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDec, 2);
    wsprintf(szIn,TEXT("%ld"),i);
    if (0 == GetNumberFormat( LOCALE_USER_DEFAULT, 0, szIn, NULL, szOut, 32 ))
    {
        _tcscpy( pszDest, szIn );
        return FALSE;
    }
    pszDec = _tcsrchr(szOut, szDec[0]);
    *pszDec = '\0';
    return (NULL != _tcscpy( pszDest, szOut ) );
}


BOOL PrintValueLine(TCHAR * szText, DWORD dwValue, PRINTCBINFO *lpInfo)
{
    TCHAR    szBuff[80];
    DWORD cchLen;
    int xName, xVal, yLine;

    // Calculate Name and Value column x offsets
    xName   = (lpInfo->dwCurrIndent * DEF_TAB_SIZE * lpInfo->dwCharWidth);
    xVal    = xName + (32 * lpInfo->dwCharWidth);
    yLine   = (lpInfo->dwCurrLine * lpInfo->dwLineHeight);

    // Print name
    wsprintf (szBuff, szText);
    cchLen = _tcslen (szText);
    if (! PrintLine (xName, yLine, szBuff, cchLen, lpInfo))
        return FALSE;

    // Print value
    Int2Str(szBuff,dwValue);
    cchLen = _tcslen (szBuff);
    if (! PrintLine (xVal, yLine, szBuff, cchLen, lpInfo))
        return FALSE;

    // Advance to next line on page
    if (! PrintNextLine (lpInfo))
        return FALSE;

   return TRUE;
}


//================================================================
//  About - process about box
//================================================================
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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


//================================================================
//================================================================
//  DXView_OnSize
//
//  DESCRIPTION:
//     Called whenever the size of the app window has changed or the size
//     of its child controls should be adjusted.
//
//  PARAMETERS:
//     hWnd, handle of app window.
//
//================================================================
//================================================================
void DXView_OnSize(HWND hWnd)
{
    HDWP hDWP;
    RECT ClientRect;
    int Height;
    HWND hKeyTreeWnd, hValueListWnd;
    int x;
    int dx;

    if ((hDWP = BeginDeferWindowPos(2)) != NULL)
    {
        GetClientRect(hWnd, &ClientRect);

        // Account for height and position of command bar when positioning
        // these child windows.  The command bar must sit at 0,0 and not
        // be obscured by other windows.
        Height = ClientRect.bottom - ClientRect.top - g_nCmdBarHeight;

        hKeyTreeWnd = g_hwndTV;

        DeferWindowPos(hDWP, hKeyTreeWnd, NULL,
                       0,
                       ClientRect.top + g_nCmdBarHeight,
                       g_xPaneSplit,
                       Height,
                       SWP_NOZORDER | SWP_NOACTIVATE);

        x = g_xPaneSplit + GetSystemMetrics(SM_CXBORDER);
        dx = ClientRect.right - ClientRect.left - x;

        hValueListWnd = g_hwndLV;

        DeferWindowPos(hDWP, hValueListWnd, NULL,
                       x,
                       ClientRect.top + g_nCmdBarHeight,
                       dx,
                       Height,
                       SWP_NOZORDER | SWP_NOACTIVATE);

        EndDeferWindowPos(hDWP);
    }
}

#ifdef _X86_
#pragma optimize("", off)
#endif

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
HTREEITEM TVAddNode(
    HTREEITEM       hParent,
    LPTSTR           szText,
    BOOL            fKids,
    int             iImage,
    SELCALLBACK     Callback,
    LPARAM          lParam1,
    LPARAM          lParam2,
    PRINTCALLBACK   printCallback)
{
    TV_INSERTSTRUCT tvi;
    NODEINFO *pni;

    pni = (NODEINFO *)LocalAlloc(LPTR, sizeof(NODEINFO));

    if (pni == NULL)
        return NULL;

    pni->lParam1        = lParam1;
    pni->lParam2        = lParam2;
    pni->Callback       = Callback;
    pni->printCallback  = printCallback;

    // Add Node to treeview
    tvi.hParent             = hParent;
    tvi.hInsertAfter        = TVI_LAST;
    tvi.item.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
                              TVIF_PARAM | TVIF_CHILDREN;
    tvi.item.iImage         = iImage - IDI_FIRSTIMAGE;
    tvi.item.iSelectedImage = iImage - IDI_FIRSTIMAGE;
    tvi.item.lParam         = (LPARAM)pni;
    tvi.item.cChildren      = fKids;
    tvi.item.pszText        = szText;

    return TreeView_InsertItem(g_hwndTV, &tvi);
}

#ifdef _X86_
#pragma optimize("", on)
#endif
