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
//   Movie player console application
//
//   Function:
//       Plays a single movie file. Supports recursion, wildcards, and
//       automatic playing of all supported media files for a given system.
//
//       See Usage() or readme.txt for detail.
//

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include <control.h>
#include <stdio.h>
#include <vfwmsgs.h>
#include <evcode.h>

#include "playall.h"
#include "playvideo.h"

//
// Function prototypes
//
void DisplayECEvent(long lEventCode);
void ResetDirectShow();
void CenterVideo(void);
void GetFilename(TCHAR *pszFull, TCHAR *pszFile);
void ShowCurrentFile(LPTSTR lpszMovie);

//
// Global data
//
unsigned int g_nIterations=0, g_nLoopCount=0;



HRESULT PlayFile(LPTSTR pszFile, int nPlayCount)
{
    HRESULT hr=S_OK;
    BOOL bComplete=0;

    // Prepare the movie for playback
    hr = PrepareMedia(pszFile, nPlayCount);

    if (SUCCEEDED(hr))
    {
        // Start the movie playing now
        hr = RunMedia();
        if (FAILED(hr))
            return hr;

        // Increment global count
        g_nFilesPlayed++;

        // Monitor DirectShow events until complete AND all requested
        // loops have been played.
        while (TRUE)
        {
            // Give the DirectShow code a chance to check for events
            hr = CheckMovieState(&bComplete);
            if (FAILED(hr))
            {
                RetailOutput(TEXT("Failure while checking movie state.  Playback will end.\r\n"));
                ResetDirectShow();
                break;
            }

            // Have we played through all of the requested loops?
            if (bComplete)
            {
                ResetDirectShow();
                break;
            }

            Sleep(10);
        }
    }

    return(hr);
}


HRESULT PrepareMedia(LPTSTR lpszMovie, unsigned int nLoopCount)
{
    HRESULT hr = S_OK;

    // Allow DirectShow to create the FilterGraph for this media file
    hr = pGB->RenderFile(lpszMovie, NULL);
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in RenderFile(%s)!\r\n"),
                 hr, lpszMovie);

        // Increment global count of fails failing to render
        g_nFilesFailed++;
        return hr;
    }

    // Set video to full screen size if requested
    if (g_bFullScreen)
    {
        hr = pVW->put_FullScreenMode(OATRUE);

        if (FAILED(hr))
        {
            hr = pBV->SetDestinationPosition(0, 0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN));
            if (FAILED(hr)) {
                if (hr == E_NOINTERFACE)
                {
                    RetailOutput(TEXT("    -- No video interface. Assuming AUDIO file or unsupported video codec.\r\n"));
                    g_nAudioOnly++;
                }
                else
                {
                    RetailOutput(TEXT("\r\n*** Failed(%08lx) in SetDestinationPosition()! (FullScreen)\r\n"),
                             hr);
                    return hr;
                }
            }
        }
    }
    else
    {
        CenterVideo();
    }

    ShowCurrentFile(lpszMovie);

    // Display first frame of the movie
    hr = pMC->Pause();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Pause()!\r\n"), hr);
        return hr;
    }

    // Initialize state variables
    g_psCurrent = psSTOPPED;
    g_vsCurrent = vsDEFAULT;

    // Initialize counters
    g_nLoopCount = nLoopCount;
    g_nIterations = 0;

    RetailOutput(TEXT("    Playing %s for %d iteration%s\r\n"), lpszMovie,
                 g_nLoopCount,
                 (g_nLoopCount > 1) ? TEXT("s.") : TEXT("."));

    return hr;
}


HRESULT CheckMovieState(BOOL *pbComplete)
{
    HRESULT hr = S_OK;

    if (g_bContinue)
    {
        MSG msg;
        LONGLONG pos=0;
        long lEventCode, lParam1, lParam2;

        // Has there been a media event?  Look for end of stream condition.
        while(E_ABORT != pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0))
        {
            // Free the media event resources.
            hr = pME->FreeEventParams(lEventCode, lParam1, lParam2);
            if (FAILED(hr))
            {
                RetailOutput(TEXT("\r\n*** Failed(%08lx) to free event params!\r\n"), hr);
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
                        StopMedia();

                        // Since we're looping, a failure to reset to the beginning
                        // would cause the app to lockup, since it would wait
                        // forever for the requested number of loops to complete.
                        // In that case, just leave the while() loop and return.
                        if ((hr = RunMedia()) != S_OK)
                        {
                            RetailOutput(TEXT("\r\n*** Failed(%08lx) to reset media clip!\r\n"), hr);
                            *pbComplete = TRUE;
                            break;
                        }
                    }

                    // Increment loop count for next pass
                    g_nIterations++;
                    if (g_nLoopCount > 1)  // Only show reset msg for multiple loops
                    {
                        RetailOutput(TEXT("---  Reset media file to beginning (Iteration #%d of %d)... ---\r\n"),
                            g_nIterations, g_nLoopCount);
                    }
                }

                if (g_nIterations >= g_nLoopCount)
                {
                    *pbComplete = TRUE;
                }
            }

            //
            //  If requested, display DirectShow events received
            //
            else if (g_bDisplayEvents)
            {
                DisplayECEvent(lEventCode);
            }
        }

        // Check and process window messages (like our keystrokes)
        while (PeekMessage (&msg, g_hwndMain, 0, 0, PM_NOREMOVE))
        {
            if (!GetMessage (&msg, g_hwndMain, 0, 0))
                return 1;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return hr;
}


void CenterVideo(void)
{
    LONG width, height;
    int nCX, nCY;
    HRESULT hr;

    hr = pVW->get_Width(&width);
    hr = pVW->get_Height(&height);
    nCX = (g_nMaxX / 2) - (width / 2);
    nCY = (g_nMaxY / 2) - (height / 2);

    hr = pVW->SetWindowPosition(nCX, nCY, width, height);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to set window position!  hr=0x%x\r\n"), hr);
        return;
    }
}


HRESULT RunMedia()
{
    HRESULT hr = S_OK;

    if (!pMC)
        return hr;

    // Start playback
    hr = pMC->Run();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Run()!\r\n"), hr);
        return hr;
    }

    // Update state variables
    g_psCurrent = psRUNNING;
    g_bContinue = TRUE;
    g_bLooping = TRUE;

    return hr;
}


HRESULT StopMedia()
{
    HRESULT hr = S_OK;

    if (!pMC)
        return hr;

    // If we're already stopped, don't check again
    if (g_psCurrent == psSTOPPED)
        return hr;

    // Stop playback
    hr = pMC->Stop();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Stop()!\r\n"), hr);
        return hr;
    }

    // Update state variables
    g_psCurrent = psSTOPPED;
    g_bContinue = FALSE;
    g_bLooping = FALSE;

    return hr;
}


void ShowCurrentFile(LPTSTR lpszMovie)
{
    TCHAR szFile[MAX_PATH], szTitle[MAX_PATH];
    BSTR bstrFile;

    GetFilename(lpszMovie, szFile);
    wsprintf(szTitle, TEXT("%s - %s\0"), NAME, szFile);

    // Set the window's taskbar caption
    bstrFile = SysAllocString(szTitle);
    pVW->put_Caption(bstrFile);
    SysFreeString(bstrFile);
}


void GetFilename(TCHAR *pszFull, TCHAR *pszFile)
{
    int nLength;
    TCHAR szPath[MAX_PATH];

    _tcscpy(szPath, pszFull);
    nLength = _tcslen(szPath);

    for (int i=nLength-1; i; i--)
    {
        if (szPath[i] == '\\')
        {
            szPath[i] = '\0';
            lstrcpy(pszFile, &szPath[i+1]);
            break;
        }
    }
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
        RetailOutput(TEXT("  Received unknown event code (0x%x)\r\n"), lEventCode);
        break;
    }
}


HRESULT InitDirectShow(void)
{
    HRESULT hr = S_OK;

    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pGB));
    JIF(pGB->QueryInterface(IID_IMediaControl,  (void **)&pMC));
    JIF(pGB->QueryInterface(IID_IMediaSeeking,  (void **)&pMS));
    JIF(pGB->QueryInterface(IID_IBasicVideo,    (void **)&pBV));
    JIF(pGB->QueryInterface(IID_IVideoWindow,   (void **)&pVW));
    JIF(pGB->QueryInterface(IID_IMediaEventEx,  (void **)&pME));

    return S_OK;

CLEANUP:
    FreeDirectShow();
    return(hr);
}


HRESULT FreeDirectShow(void)
{
    HRESULT hr=S_OK;

    StopMedia();

    DestroyWindow(g_hwndMain);

    RELEASE(pGB);
    RELEASE(pMC);
    RELEASE(pMS);
    RELEASE(pVW);
    RELEASE(pBV);
    RELEASE(pME);

    return hr;
}


void ResetDirectShow()
{
    StopMedia();
    FreeDirectShow();
    InitDirectShow();
}

