//-----------------------------------------------------------------------------
// File: DDEx4.CPP
//
// Desc: Direct Draw example program 4.  Adds functionality to 
//       example program 3.  Creates a flipping surface and loads
//       a bitmap image into an offscreen surface.  Uses BltFast to
//       copy portions of the offscreen surface to the back buffer
//       to generate an animation.  Illustrates watching return
//       code from BltFast to prevent image tearing.  This program
//       requires 1.2 Meg of video ram.
//
// Copyright (c) 1995-2000 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include <windows.h>
#include <ddraw.h>
#include "resource.h"
#include "ddutil.h"

//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------
#define NAME                TEXT("DDExample4")
#define TITLE               TEXT("Direct Draw Example 4")

//-----------------------------------------------------------------------------
// Default settings
//-----------------------------------------------------------------------------
#define TIMER_ID            1
#define TIMER_RATE          18

//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------
LPDIRECTDRAW4               g_pDD = NULL;        // DirectDraw object
LPDIRECTDRAWSURFACE4        g_pDDSPrimary = NULL;// DirectDraw primary surface
LPDIRECTDRAWSURFACE4        g_pDDSBack = NULL;   // DirectDraw back surface
LPDIRECTDRAWSURFACE4        g_pDDSOne = NULL;    // Offscreen surface 1
BOOL                        g_bActive = FALSE;   // Is application active?

//-----------------------------------------------------------------------------
// Local data
//-----------------------------------------------------------------------------
static TCHAR                szBitmap[] = TEXT("ALL");
static HINSTANCE            hInstance;



//-----------------------------------------------------------------------------
// Name: ReleaseAllObjects()
// Desc: Finished with all objects we use; release them
//-----------------------------------------------------------------------------
static void
ReleaseAllObjects(void)
{
    if (g_pDDSBack != NULL)
    {
        g_pDDSBack->Release();
        g_pDDSBack = NULL;
    }
    if (g_pDDSPrimary != NULL)
    {
        g_pDDSPrimary->Release();
        g_pDDSPrimary = NULL;
    }
    if (g_pDDSOne != NULL)
    {
        g_pDDSOne->Release();
        g_pDDSOne = NULL;
    }
    if (g_pDD != NULL)
    {   
        g_pDD->Release();
        g_pDD = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: InitFail()
// Desc: This function is called if an initialization function fails
//-----------------------------------------------------------------------------
#define PREFIX      TEXT("DDEX4: ")
#define PREFIX_LEN  7

HRESULT
InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError,...)
{
    TCHAR                       szBuff[128];
    va_list                     vl;

    va_start(vl, szError);
    wsprintf(szBuff, PREFIX);
    wvsprintf(szBuff + PREFIX_LEN, szError, vl);
    ReleaseAllObjects();
    OutputDebugString(szBuff);
    DestroyWindow(hWnd);
    va_end(vl);
    return hRet;
}

#undef PREFIX_LEN
#undef PREFIX


//-----------------------------------------------------------------------------
// Name: RestoreAll()
// Desc: Restore all lost objects
//-----------------------------------------------------------------------------
HRESULT 
RestoreAll(void)
{
    HRESULT                     hRet;

    hRet = g_pDDSPrimary->Restore();
    if (hRet == DD_OK)
    {
        hRet = g_pDDSOne->Restore();
        if (hRet == DD_OK)
        {
            DDReLoadBitmap(hInstance, g_pDDSOne, szBitmap);
        }
    }
    return hRet;
}




//-----------------------------------------------------------------------------
// Name: UpdateFrame()
// Desc: Decide what needs to be blitted next, wait for flip to complete,
//       then flip the buffers.
//-----------------------------------------------------------------------------
void 
UpdateFrame(void)
{
    static DWORD                lastTickCount[3] =  {0, 0, 0};
    static int                  currentFrame[3] =   {0, 0, 0};
    DWORD                       delay[3] =          {50, 78, 13};
    int                         xpos[3] =           {288, 190, 416};
    int                         ypos[3] =           {128, 300, 256};
    int                         i;
    DWORD                       thisTickCount;
    RECT                        rcRect;
    HRESULT                     hRet;

    // Decide which frame will be blitted next
    thisTickCount = GetTickCount();
    for (i = 0; i < 3; i++)
    {
        if ((thisTickCount - lastTickCount[i]) > delay[i])
        {
            // Move to next frame;
            lastTickCount[i] = thisTickCount;
            currentFrame[i]++;
            if (currentFrame[i] > 59)
                currentFrame[i] = 0;
        }
    }

    // Blit the stuff for the next frame
    rcRect.left = 0;
    rcRect.top = 0;
    rcRect.right = 640;
    rcRect.bottom = 480;
    while (TRUE)
    {
        hRet = g_pDDSBack->BltFast(0, 0, g_pDDSOne,
                                    &rcRect, DDBLTFAST_NOCOLORKEY);

        if (hRet == DD_OK)
            break;
        if (hRet == DDERR_SURFACELOST)
        {
            hRet = RestoreAll();
            if (hRet != DD_OK)
                return;
        }
        if (hRet != DDERR_WASSTILLDRAWING)
            return;
    }
    if (hRet != DD_OK)
        return;

    for (i = 0; i < 3; i++)
    {
        rcRect.left = currentFrame[i] % 10 * 64;
        rcRect.top = currentFrame[i] / 10 * 64 + 480;
        rcRect.right = currentFrame[i] % 10 * 64 + 64;
        rcRect.bottom = currentFrame[i] / 10 * 64 + 64 + 480;

        while (TRUE)
        {
            hRet = g_pDDSBack->BltFast(xpos[i], ypos[i], g_pDDSOne,
                                        &rcRect, DDBLTFAST_SRCCOLORKEY);
            if (hRet == DD_OK)
                break;
            if (hRet == DDERR_SURFACELOST)
            {
                hRet = RestoreAll();
                if (hRet != DD_OK)
                    return;
            }
            if (hRet != DDERR_WASSTILLDRAWING)
                return;
        }
    }

    // Flip the surfaces
    while (TRUE)
    {
        hRet = g_pDDSPrimary->Flip(NULL, 0);
        if (hRet == DD_OK)
            break;
        if (hRet == DDERR_SURFACELOST)
        {
            hRet = RestoreAll();
            if (hRet != DD_OK)
                break;
        }
        if (hRet != DDERR_WASSTILLDRAWING)
            break;
    }
}




//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The Main Window Procedure
//-----------------------------------------------------------------------------
long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
#ifdef UNDER_CE
        case WM_ACTIVATE:
#else
        case WM_ACTIVATEAPP:
#endif
            // Pause if minimized or not the top window
            g_bActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
            return 0L;

        case WM_DESTROY:
            // Clean up and close the app
            ReleaseAllObjects();
            PostQuitMessage(0);
            return 0L;

        case WM_KEYDOWN:
            // Handle any non-accelerated key commands
            switch (wParam)
            {
                case VK_ESCAPE:
                case VK_F12:
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    return 0L;
            }
            break;

        case WM_SETCURSOR:
            // Turn off the cursor since this is a full-screen app
            SetCursor(NULL);
            return TRUE;

        case WM_TIMER:
	        // Shoot another frame.
	        UpdateFrame();
	        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: InitApp()
// Desc: Do work required for every instance of the application:
//          Create the window, initialize data
//-----------------------------------------------------------------------------
static HRESULT
InitApp(int nCmdShow)
{
    HWND                        hWnd;
    WNDCLASS                    wc;
    DDSURFACEDESC2              ddsd;
    DDSCAPS2                    ddscaps;
    HRESULT                     hRet;
    LPDIRECTDRAW                pDD;

    // Set up and register window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = NAME;
    RegisterClass(&wc);

    // Create a window
    hWnd = CreateWindowEx(WS_EX_TOPMOST,
                          NAME,
                          TITLE,
                          WS_POPUP,
                          0,
                          0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN),
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    if (!hWnd)
        return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    SetFocus(hWnd);

    ///////////////////////////////////////////////////////////////////////////
    // Create the main DirectDraw object
    ///////////////////////////////////////////////////////////////////////////
    hRet = DirectDrawCreate(NULL, &pDD, NULL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("DirectDrawCreate FAILED"));

    // Fetch DirectDraw4 interface and release IDirectDraw.
    hRet = pDD->QueryInterface(IID_IDirectDraw4, (LPVOID *) & g_pDD);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("QueryInterface FAILED"));
    pDD->Release();
    pDD = NULL;

    // Get exclusive mode
    hRet = g_pDD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("SetCooperativeLevel FAILED"));

    // Set the video mode to 640x480x8

#ifndef UNDER_CE
    hRet = g_pDD->SetDisplayMode(640, 480, 8, 0, 0);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("SetDisplayMode FAILED"));
#endif

    // Create the primary surface with 1 back buffer
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP |
                          DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;
    hRet = g_pDD->CreateSurface(&ddsd, &g_pDDSPrimary, NULL);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("CreateSurface FAILED"));

    // Get a pointer to the back buffer
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    hRet = g_pDDSPrimary->GetAttachedSurface(&ddscaps, &g_pDDSBack);
    if (hRet != DD_OK)
        return InitFail(hWnd, hRet, TEXT("GetAttachedSurface FAILED"));

    // Create the offscreen surface, by loading our bitmap.
    g_pDDSOne = DDLoadBitmap(hInstance, g_pDD, szBitmap);
    if (g_pDDSOne == NULL)
        return InitFail(hWnd, hRet, TEXT("DDLoadBitmap FAILED"));

    // Set the color key for this bitmap (black)
    DDSetColorKey(g_pDDSOne, RGB(0, 0, 0));

    // Create a timer to flip the pages
    if (TIMER_ID != SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL))
        return InitFail(hWnd, hRet, TEXT("SetTimer FAILED"));

    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Initialization, message loop
//-----------------------------------------------------------------------------
int PASCAL
WinMain(HINSTANCE hCurInstance,
        HINSTANCE hPrevInstance,
#ifdef UNDER_CE
        LPWSTR lpCmdLine,
#else
        LPSTR lpCmdLine,
#endif
        int nCmdShow)
{
    MSG                         msg;

    hInstance = hCurInstance;

    if (InitApp(nCmdShow) != DD_OK)
        return FALSE;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

