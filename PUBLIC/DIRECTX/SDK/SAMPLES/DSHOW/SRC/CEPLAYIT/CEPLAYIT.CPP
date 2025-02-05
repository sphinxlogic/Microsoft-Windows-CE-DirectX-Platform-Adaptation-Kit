//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 1999  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
//
//   Simple movie player console application
//
//   Function:
//       Plays a movie file. Support simple user input to enable:
//          - Toggle full screen / normal size
//          - Pause / Stop
//          - Replay from beginning
//          - Loop flag (continues playing until Stop or reset loop flag)
//          - Move video window around the screen
//          - Stretch / shrink video window
//
//       See Usage() or readme.txt for detail.
//
//   Usage: ceplayit <media file>
//       The media file may exist in the image, on a network server, or on
//       the attached development machine in the _FLATRELEASEDIR directory.
//

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include <control.h>
#include <stdio.h>
#include <vfwmsgs.h>
#include <evcode.h>
#include <amvideo.h>

#include "ceplayit.h"

// Constants
#define MAX_ARGS    30

// Globals
BOOL g_bKeepCESHFile=FALSE;

// Function prototypes
void DisplayECEvent(long lEventCode);
long ProcessVideoKey(WORD wParam);
long ProcessAudioKey(WORD wParam);


//
// main program code
//
int
APIENTRY
WinMain (
         HINSTANCE hInstance,
         HINSTANCE hPrevInstance,
         LPTSTR lpCmdLine,
         int nCmdShow
         )
{
    TCHAR szFile[MAX_PATH]={0}, szCmd[MAX_PATH]={0}, szOpt[MAX_PATH]={0};

    // Copy command line for manipulation
    lstrcpy (szFile, lpCmdLine);
    ParseOptions(szFile, szCmd, szOpt);

    if (! ConvertFilename(szFile, szCmd))
        return 1;
    if (!ParseCommandLine(szOpt))
        return 1;

    RETAILMSG(1, (TEXT("CEPlayit: Playing file %s.\r\n"), szFile));

    // Create the main window
    if (!DoInit(hInstance, nCmdShow, szFile))
        return 1;

    // Get COM interfaces
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(GetInterfaces()))
    {
        CoUninitialize();
        return 1;
    }

    // Now play the movie
    PlayMedia(szFile, hInstance);

    // Release COM interfaces
    CleanupInterfaces();
    CoUninitialize();

    // If a temporary file was created (for CESH source), delete it
    if (!g_bKeepCESHFile)
        DeleteTemporaryFile();

    OutputDebugString(TEXT("CEPlayit exiting.\r\n\r\n"));
    return 0;
}


long FAR PASCAL WindowProc( HWND hWnd, UINT message,
                           WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
        // Monitor keystrokes for manipulating video window
        // and program options
    case WM_KEYDOWN:
        if (g_bAudioOnly)
            ProcessAudioKey(wParam);
        else
            ProcessVideoKey(wParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


long ProcessAudioKey(WORD wParam)
{
    switch( wParam )
    {
    case VK_ESCAPE:
    case VK_F12:
    case 'Q':
    case 'X':
        // Quit
        pMC->Stop();
        g_psCurrent = psSTOPPED;
        PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
        g_bContinue = FALSE;
        g_bLooping = FALSE;
        break;

    case 'P':
        // Play/pause
        if ((g_psCurrent == psPAUSED) || (g_psCurrent == psSTOPPED))
        {
            pMC->Run();
            g_psCurrent = psRUNNING;
            OutputDebugString(TEXT("CEPlayit: Running\r\n"));
        }
        else
        {
            pMC->Pause();
            g_psCurrent = psPAUSED;
            OutputDebugString(TEXT("CEPlayit: Paused\r\n"));
        }
        break;

    case 'S':
        // Stop and reset position to beginning
        if ((g_psCurrent == psPAUSED) || (g_psCurrent == psRUNNING))
        {
            LONGLONG pos = 0;
            pMC->Stop();
            pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                NULL, AM_SEEKING_NoPositioning);
            g_psCurrent = psSTOPPED;

            // Display the first frame to indicate the reset condition
            pMC->Pause();
            OutputDebugString(TEXT("CEPlayit: Stopped\r\n"));
        }
        break;

    case 'R':
    case VK_F10:
        // Reset position to beginning and play
        ResetAudio();
        break;

    case 'L':
        // Modify loop flag
        g_bLooping ^= 1;
        RETAILMSG(1, (TEXT("CEPlayit: Looping is %s.\r\n"),
            g_bLooping ? TEXT("On") : TEXT("Off")));
        break;
    }

    return 0;
}


long ProcessVideoKey(WORD wParam)
{
    switch( wParam )
    {
    case VK_ESCAPE:
    case VK_F12:
    case 'Q':
    case 'X':
        // Quit
        pMC->Stop();
        g_psCurrent = psSTOPPED;
        PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
        g_bContinue = FALSE;
        g_bLooping = FALSE;
        break;

    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        ProcessMoveKeysVW(wParam);
        break;

    case 'P':
        // Play/pause
        if ((g_psCurrent == psPAUSED) || (g_psCurrent == psSTOPPED))
        {
            pMC->Run();
            g_psCurrent = psRUNNING;
            OutputDebugString(TEXT("CEPlayit: Running\r\n"));
        }
        else
        {
            pMC->Pause();
            g_psCurrent = psPAUSED;
            OutputDebugString(TEXT("CEPlayit: Paused\r\n"));
        }
        break;

    case 'S':
        // Stop and reset postion to beginning
        if ((g_psCurrent == psPAUSED) || (g_psCurrent == psRUNNING))
        {
            LONGLONG pos = 0;
            pMC->Stop();
            pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                NULL, AM_SEEKING_NoPositioning);
            g_psCurrent = psSTOPPED;

            // Display the first frame to indicate the reset condition
            pMC->Pause();
            OutputDebugString(TEXT("CEPlayit: Stopped\r\n"));
        }
        break;

    case 'R':
        // Reset position to beginning and play
        ResetVideo();
        break;

    case 'L':
        // Modify loop flag
        g_bLooping ^= 1;
        RETAILMSG(1, (TEXT("CEPlayit: Looping is %s.\r\n"),
            g_bLooping ? TEXT("On") : TEXT("Off")));
        break;

    case 'F':
        ToggleFullscreen();
        break;

    case VK_ADD:
    case VK_EQUAL:
        ModifyVideoSize(UP);
        break;

    case VK_SUBTRACT:
    case VK_HYPHEN:
        ModifyVideoSize(DOWN);
        break;

    case VK_F1:
    case VK_F2:
    case VK_F3:
    case VK_F4:
    case VK_F5:
    case VK_F6:
    case VK_F7:
    case VK_F8:
    case VK_F9:
        SetVideoQuadrant(wParam - VK_F1);
        break;

    case VK_F10:
        ResetVideo();
        CenterVideo();
        break;

    case 'N':
        // Reset position to ideal minimum coordinates
        ResetVideoMinimum();
        break;

    case 'M':
        // Reset position to ideal maximum coordinates
        ResetVideoMaximum();
        break;

    case 'C':
        CenterVideo();
        break;
    }

    return 0;
}


HRESULT AddWndlessRendererToGraph(IGraphBuilder *pGB)
{
    HRESULT hr;

    JIF(CoCreateInstance(
        CLSID_OverlayMixer,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void **) &pBF_OvM));

    JIF(pGB->AddFilter(pBF_OvM, L"Windowless Video Renderer"));

CLEANUP:
    return hr;
}


HRESULT PlayMedia(LPTSTR lpszMovie, HINSTANCE hInstance)
{
    HRESULT hr = S_OK;
    TCHAR buf[256];
    BOOL bSleep=TRUE;

    if (g_bWindowless)
    {
        JIF(AddWndlessRendererToGraph(pGB));
    }

    // Allow DirectShow to create the FilterGraph for this media file
    hr = pGB->RenderFile(lpszMovie, NULL);
    if (FAILED(hr)) {
        wsprintf(buf, TEXT("Failed(%08lx) in RenderFile(%s)!\r\n"),
            hr, lpszMovie);
        OutputDebugString(buf);
        return hr;
    }

    // Display first frame of the movie
    hr = pMC->Pause();
    if (FAILED(hr)) {
        wsprintf(buf, TEXT("Failed(%08lx) in Pause()!\r\n"), hr);
        OutputDebugString(buf);
        return hr;
    }

    // Initialize state variables
    g_psCurrent = psSTOPPED;
    g_vsCurrent = vsDEFAULT;

    // Override defaults with command line options
    ProcessStartupOptions();

    // Get focus for keyboard input
    SetFocus(g_hwndMain);
    SetForegroundWindow(g_hwndMain);

    // Start playback
    hr = pMC->Run();
    if (FAILED(hr)) {
        wsprintf(buf, TEXT("Failed(%08lx) in Run()!\r\n"), hr);
        OutputDebugString(buf);
        return hr;
    }

    // Update state variables
    g_psCurrent = psRUNNING;
    g_bContinue = TRUE;

    // Continue
    while (g_bContinue)
    {
        MSG msg;
        LONGLONG pos=0;
        long lEventCode, lParam1, lParam2;

        // Reset sleep flag
        bSleep = TRUE;

        // Has there been a media event?  Look for end of stream condition.
        if(E_ABORT != pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0))
        {
            // Free the media event resources.
            hr = pME->FreeEventParams(lEventCode, lParam1, lParam2);
            if (FAILED(hr))
            {
                wsprintf(buf, TEXT("Failed(%08lx) to free event params (%s)!\r\n"),
                    hr, lpszMovie);
                OutputDebugString(buf);
            }

            // Is this the end of the movie?
            if (lEventCode == EC_COMPLETE)
            {
                // If user wants continuous looping, reset and continue
                if (g_bLooping)
                {
                    // Reset to first frame of movie
                    hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                        NULL, AM_SEEKING_NoPositioning);

                    if (FAILED(hr))
                    {
                        // Some filters (like MIDI) may not implement interfaces
                        // for IMediaSeeking to allow seek to the start.  In that
                        // case, just stop and restart for the same effect.
                        hr = pMC->Stop();
                        if (FAILED(hr))
                        {
                            wsprintf(buf, TEXT("Failed(%08lx) to stop media (%s)!\r\n"),
                                hr, lpszMovie);
                            OutputDebugString(buf);
                        }

                        hr = pMC->Run();
                        if (FAILED(hr))
                        {
                            wsprintf(buf, TEXT("Failed(%08lx) to restart media (%s)!\r\n"),
                                hr, lpszMovie);
                            OutputDebugString(buf);
                        }
                    }

                    // Don't sleep if we're resetting clip (for faster restart)
                    bSleep = FALSE;
                }
            }
            else
            {
                DisplayECEvent(lEventCode);
            }
        }

        // Give system threads time to run (and don't sample user input madly)
        if (bSleep)
            Sleep(SAMPLE_FREQ);

        // Check and process window messages (like our keystrokes)
        while (PeekMessage (&msg, g_hwndMain, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

CLEANUP:
    return hr;
}


void DisplayECEvent(long lEventCode)
{
    switch (lEventCode)
    {
        HANDLE_EC(EC_COMPLETE);
        HANDLE_EC(EC_USERABORT);
        HANDLE_EC(EC_ERRORABORT);
        HANDLE_EC(EC_TIME);
        HANDLE_EC(EC_REPAINT);
        HANDLE_EC(EC_STREAM_ERROR_STOPPED);
        HANDLE_EC(EC_STREAM_ERROR_STILLPLAYING);
        HANDLE_EC(EC_ERROR_STILLPLAYING);
        HANDLE_EC(EC_PALETTE_CHANGED);
        HANDLE_EC(EC_VIDEO_SIZE_CHANGED);
        HANDLE_EC(EC_QUALITY_CHANGE);
        HANDLE_EC(EC_SHUTTING_DOWN);
        HANDLE_EC(EC_CLOCK_CHANGED);
        HANDLE_EC(EC_PAUSED);
        HANDLE_EC(EC_OPENING_FILE);
        HANDLE_EC(EC_BUFFERING_DATA);
        HANDLE_EC(EC_FULLSCREEN_LOST);
        HANDLE_EC(EC_ACTIVATE);
        HANDLE_EC(EC_NEED_RESTART);
        HANDLE_EC(EC_WINDOW_DESTROYED);
        HANDLE_EC(EC_DISPLAY_CHANGED);
        HANDLE_EC(EC_STARVATION);
        HANDLE_EC(EC_OLE_EVENT);
        HANDLE_EC(EC_NOTIFY_WINDOW);
        HANDLE_EC(EC_STREAM_CONTROL_STOPPED);
        HANDLE_EC(EC_STREAM_CONTROL_STARTED);
        HANDLE_EC(EC_END_OF_SEGMENT);
        HANDLE_EC(EC_SEGMENT_STARTED);
        HANDLE_EC(EC_LENGTH_CHANGED);
        HANDLE_EC(EC_TIMECODE_AVAILABLE);
        HANDLE_EC(EC_EXTDEVICE_MODE_CHANGE);
        HANDLE_EC(EC_VCD_SELECT);
        HANDLE_EC(EC_VCD_PLAY);
        HANDLE_EC(EC_VCD_END);

    default:
        RETAILMSG(1, (TEXT("  Received unknown event code (0x%x)\r\n"), lEventCode));
        break;
    }
}


BOOL DoInit( HINSTANCE hInstance, int nCmdShow, TCHAR *szFile )
{
    ATOM     atomWndClass;
    TCHAR    szTitle[MAX_PATH];

    // set up and register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = NAME;
    atomWndClass = RegisterClass( &wc );

    wsprintf(szTitle, TEXT("%s: %s"), NAME, szFile);

    // Get max screen coordinates
    g_nMaxX = GetSystemMetrics(SM_CXSCREEN);
    g_nMaxY = GetSystemMetrics(SM_CYSCREEN);

    // Create a window of zero size that will serve as the sink for
    // keyboard input.  If this media file has a video component, then
    // a second ActiveMovie window will be displayed in which the video
    // will be rendered.  Setting keyboard focus on this application window
    // will allow the user to move the video window around the screen, make
    // it full screen, resize, center, etc. independent of the application
    // window.  If the media file has only an audio component, then this will
    // be the only window created.
    g_hwndMain = CreateWindowEx(
        0,
        NAME,
        szTitle,
        WS_VISIBLE,
        0, 0,
        0, 0,
        NULL,
        NULL,
        hInstance,
        NULL );

    return (g_hwndMain != NULL);
}


HRESULT GetInterfaces(void)
{
    HRESULT hr = S_OK;

    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pGB));
    JIF(pGB->QueryInterface(IID_IMediaControl,  (void **)&pMC));
    JIF(pGB->QueryInterface(IID_IMediaSeeking,  (void **)&pMS));
    JIF(pGB->QueryInterface(IID_IMediaPosition, (void **)&pMP));
    JIF(pGB->QueryInterface(IID_IVideoWindow,   (void **)&pVW));
    JIF(pGB->QueryInterface(IID_IBasicVideo,    (void **)&pBV));
    JIF(pGB->QueryInterface(IID_IMediaEventEx,  (void **)&pME));

    return S_OK;

CLEANUP:
    CleanupInterfaces();
    return(hr);
}


void CleanupInterfaces(void)
{
    RELEASE(pGB);
    RELEASE(pMC);
    RELEASE(pMS);
    RELEASE(pMP);
    RELEASE(pVW);
    RELEASE(pBV);
    RELEASE(pME);
    RELEASE(pBF_OvM);

    DestroyWindow(g_hwndMain);
}

void Usage(void)
{
    OutputDebugString(TEXT("\r\nUsage: CEPlayit <media file>\r\n"));
    OutputDebugString(TEXT("   Ex: CEPlayit small.avi                 (Play file in CE image)\r\n"));
    OutputDebugString(TEXT("   Ex: CEPlayit cesh:small.avi            (Play file through CESH link)\r\n"));
    OutputDebugString(TEXT("   Ex: CEPlayit http://myserver/small.avi (Play file on HTTP server)\r\n"));
    OutputDebugString(TEXT("   Ex: CEPlayit cesh:small.avi -l -x100 -y100 -w160 -h120\r\n"));
    OutputDebugString(TEXT("   Ex: CEPlayit -z                        (List all running instances)\r\n"));
    OutputDebugString(TEXT("   Ex: CEPlayit -z<file>                  (Set focus to instance playing <file>)\r\n"));

    OutputDebugString(TEXT("\r\nExclusive options:\r\n"));
    OutputDebugString(TEXT("  -z       List current running players\r\n"));
    OutputDebugString(TEXT("  -z<file> Set keyboard focus to instance playing <file>\r\n"));

    OutputDebugString(TEXT("\r\nCommand line options (any combination):\r\n"));
    OutputDebugString(TEXT("  -l    Enable looping of video file      -f    Play fullscreen\r\n"));
    OutputDebugString(TEXT("  -k    Keep copied CESH file in image    -c    Center video\r\n"));
    OutputDebugString(TEXT("  -m    Use windowless video renderer\r\n"));

    OutputDebugString(TEXT("  -x<#> Set X coordinate                  -y<#> Set Y coordinate\r\n"));
    OutputDebugString(TEXT("  -w<#> Set video width                   -h<#> Set video height\r\n"));
    OutputDebugString(TEXT("  -q<#> Initialize to given quadrant (1-10)\r\n"));

    OutputDebugString(TEXT("\r\nKeyboard controls:\r\n"));
    OutputDebugString(TEXT("  P                     Play/Pause toggle\r\n"));
    OutputDebugString(TEXT("  S                     Stop and Rewind\r\n"));
    OutputDebugString(TEXT("  R                     Reset to defaults and play from beginning\r\n"));
    OutputDebugString(TEXT("  F                     Toggle full-screen video\r\n"));
    OutputDebugString(TEXT("  L                     Toggle looping (on/off)\r\n"));
    OutputDebugString(TEXT("  C                     Center video\r\n"));
    OutputDebugString(TEXT("  Up/Down/Left/Right    Move video window\r\n"));
    OutputDebugString(TEXT("  SHIFT + arrow keys    Stretch / shrink video window\r\n"));
    OutputDebugString(TEXT("  + or -                Increase/decrease video size and center\r\n"));
    OutputDebugString(TEXT("  F1-F4                 Position video in one screen quadrant\r\n"));
    OutputDebugString(TEXT("  F5-F8                 Position video in two screen quadrants\r\n"));
    OutputDebugString(TEXT("  F9                    Position video in default web client area\r\n"));
    OutputDebugString(TEXT("  F10                   Reset to default size and center video\r\n"));
    OutputDebugString(TEXT("  Quit                  ESC or Q or X\r\n\r\n"));
}


BOOL ConvertFilename(TCHAR *szFile, LPTSTR lpCmdLine)
{
    DWORD dwAttributes;

    // Convert to lowercase for parsing
    szFile = _tcslwr(szFile);

    //
    // Allow the user to pass in just the "-z" or "-z<file>" option.
    // This overrides the normal functionality to either list all current
    // running instances of CEPlayit or to set the keyboard focus to an
    // instance that is playing the given file.
    //
    if ((_tcsstr(szFile, TEXT("-z"))) || (_tcsstr(szFile, TEXT("/z"))))
    {
        SetVideoFocus(szFile+2);
        return FALSE;
    }

    // If this is a URL, don't convert the filename
    if ((_tcsstr(szFile, TEXT("http:"))) || (_tcsstr(szFile, TEXT("https:"))))
    {
        OutputDebugString(TEXT("CEPlayit: HTTP path - Please wait while file is read from the server...\r\n"));
        return TRUE;
    }

    // Also allow for MMS: prefixes used with WMT streaming
    if ((_tcsstr(szFile, TEXT("mms:"))) || (_tcsstr(szFile, TEXT("mmsu:"))) ||
        (_tcsstr(szFile, TEXT("mmst:"))))
    {
        OutputDebugString(TEXT("CEPlayit: MMS path - Please wait while file is read from the server...\r\n"));
        return TRUE;
    }

    // If this is a file on the PC, save to temporary file on CE platform
    if ((_tcsstr(szFile, TEXT("cesh:"))) ||
        (_tcsstr(szFile, TEXT("ppsh:"))))
    {
        OutputDebugString(TEXT("CEPlayit: CESH path - Please wait while file is read from your PC...\r\n"));
        return (ReadFromPC(szFile, lpCmdLine));
    }

    dwAttributes = GetFileAttributes(szFile);

    if (dwAttributes == 0xFFFFFFFF ||
        dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        // Search for the file: assume that the given command line contains
        // a filename only (not a full path) and check for the file first
        // in the \windows directory, followed by the \cd-rom directory.
        lstrcpy(szFile, TEXT("\\Windows\\"));
        lstrcat(szFile, lpCmdLine);

        dwAttributes = GetFileAttributes(szFile);

        if (dwAttributes == 0xFFFFFFFF ||
            dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Check the root directory
            lstrcpy(szFile, TEXT("\\"));
            lstrcat(szFile, lpCmdLine);

            dwAttributes = GetFileAttributes(szFile);
            if (dwAttributes == 0xFFFFFFFF ||
                dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                Usage();
                return FALSE;
            }
        }
    }

    return TRUE;
}


void ParseOptions(TCHAR *szFile, TCHAR *szCmd, TCHAR *szOpt)
{
    // First separate filename from options
    TCHAR *pOpt = &szFile[0];

    while (*pOpt && (*pOpt != ' '))
        pOpt++;
    *pOpt = '\0';      // Null-terminate filename string
    pOpt++;            // Skip to options string(s)

    // Duplicate filename for legacy code
    lstrcpy (szCmd, szFile);

    // Fill options string with options only (no filename)
    lstrcpy (szOpt, pOpt);
}


int ParseCommandLine (TCHAR *lpOpt)
{
    TCHAR *szArg, szTok[64]={0};
    BOOL fError = FALSE;
    int counter=0;

    // Get first argument
    _tcscpy(szTok, TEXT(" "));
    szArg = _tcstok(lpOpt, szTok);

    // Return if there are no arguments to parse
    if (!szArg)
    {
        return TRUE;
    }

    // Parse command-line arguments
    do
    {
        // Check argument to ensure it is valid
        if ((*szArg != TEXT('-')) && (*szArg != TEXT('/')))
        {
            RETAILMSG(1, (TEXT("Invalid argument: %s"), szArg));
            break;
        }
        szArg++;

        // Match string with expected value
        switch (*szArg)
        {
        case TEXT('?'):
            Usage();
            fError = TRUE;
            break;

        case TEXT('L'): case TEXT('l'):
            OutputDebugString(TEXT("Looping enabled.\r\n"));
            g_bLooping = TRUE;
            break;

        case TEXT('F'): case TEXT('f'):
            // OutputDebugString(TEXT("Using FullScreen mode.\r\n"));
            g_bFullScreenStart = TRUE;
            break;

        case TEXT('C'): case TEXT('c'):
            // OutputDebugString(TEXT("Using centered window.\r\n"));
            g_bCenterStart = TRUE;
            break;

        case TEXT('M'): case TEXT('m'):
            // OutputDebugString(TEXT("Using windowless video renderer.\r\n"));
            g_bWindowless = TRUE;
            break;

        case TEXT('K'): case TEXT('k'):
            OutputDebugString(TEXT("Keeping temporary CESH file in local file system.\r\n"));
            g_bKeepCESHFile = TRUE;
            break;

        case TEXT('Q'): case TEXT('q'):
            g_wInitialQuadrant = (WORD) _ttoi(szArg+1);
            if (g_wInitialQuadrant)
            {
                // RETAILMSG(1, (TEXT("Setting quadrant to %d.\r\n"), g_wInitialQuadrant));
                g_bQuadrantStart = TRUE;
            }
            else
            {
                RETAILMSG(1, (TEXT("Invalid quadrant %s!\r\n"), szArg));
                fError = TRUE;
            }
            break;

        case TEXT('X'): case TEXT('x'):
            leftStart = _ttoi(szArg+1);
            // RETAILMSG(1, (TEXT("Setting X to %d.\r\n"), leftStart));
            break;

        case TEXT('Y'): case TEXT('y'):
            topStart = _ttoi(szArg+1);
            // RETAILMSG(1, (TEXT("Setting Y to %d.\r\n"), topStart));
            break;

        case TEXT('W'): case TEXT('w'):
            widthStart = _ttoi(szArg+1);
            // RETAILMSG(1, (TEXT("Setting width to %d.\r\n"), widthStart));
            break;

        case TEXT('H'): case TEXT('h'):
            heightStart = _ttoi(szArg+1);
            // RETAILMSG(1, (TEXT("Setting height to %d.\r\n"), heightStart));
            break;

        default:
            RETAILMSG(1, (TEXT("Unknown argument: [%s] !!!  Exiting...\r\n"), szArg-1));
            Usage();
            fError = TRUE;
            break;
        }

        szArg = _tcstok(NULL, szTok);
        counter++;
    }
    while( szArg && counter<MAX_ARGS );

    if (counter == MAX_ARGS)
    {
        RETAILMSG(1, (TEXT("Too many arguments (%d) or overflow!"), counter));
        fError = TRUE;
    }

    // Success
    return (!fError);
}



