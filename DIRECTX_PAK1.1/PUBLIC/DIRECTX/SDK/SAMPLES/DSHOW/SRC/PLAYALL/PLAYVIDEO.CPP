//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2000  Microsoft Corporation.  All Rights Reserved.
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


#define  ENFORCED_MIN_FILESIZE  400


//
// Function prototypes
//
void DisplayECEvent(long lEventCode);
void ResetDirectShow();
void CenterVideo(void);
void GetFilename(TCHAR *pszFull, TCHAR *pszFile);
void ShowCurrentVideoFile(LPTSTR lpszMovie);
void ShowCurrentAudioFile(LPTSTR lpszMovie);
BOOL ScreenFileSize(LPTSTR pszFile);
BOOL ScreenOmittedFile(LPTSTR pszFileToPlay);

//
// Global data
//
unsigned int g_nIterations=0, g_nLoopCount=0;
BOOL g_bShowingMainWindow = FALSE;



HRESULT PlayFile(LPTSTR pszFile, int nPlayCount)
{
    HRESULT hr=S_OK;
    BOOL bComplete=0;
    unsigned long ulCurrentPlayTick=0, ulMaxPlayTick=0;

//  RetailOutput(TEXT("PlayFile: %s\r\n"), pszFile);

    // Did we shut down with WM_CLOSE?
    if (g_bShutdown == TRUE)
    {
        return S_OK;
    }

    // If a command line option was used to request a skip of
    // the first N media files (to skip known good files or start
    // in the middle of a media directory, for example), then skip.
    if (g_nInitialSkipCount)
    {
        RetailOutput(TEXT("  Skipping file (#%d) (user request)\r\n"),
                     g_nSkipped);
        g_nInitialSkipCount--;
        g_nSkipped++;
        return S_OK;
    }

    // Report/check the file size.  If above the requested size, skip it.
    if (ScreenFileSize(pszFile))
    {
        g_nSkipped++;
        return S_OK;
    }

    // Is this a file we were asked to omit?
    if (ScreenOmittedFile(pszFile))
    {
        g_nSkipped++;
        return S_OK;
    }

    // Prepare the movie for playback
    hr = PrepareMedia(pszFile, nPlayCount);

    if (SUCCEEDED(hr))
    {

#ifndef SPOOF_PLAYBACK

        // If the user has asked to mute audio (by pressing "M"), then we
        // need to mute this new clip before continuing.
        if (g_bGlobalMute)
            ToggleMute();


        // Start the movie playing now
        hr = RunMedia();
        if (FAILED(hr))
            return hr;

        // Increment global count
        g_nFilesPlayed++;

        if (g_bUsingMaxPlayTime)
        {
            ulCurrentPlayTick = GetTickCount();
            ulMaxPlayTick = ulCurrentPlayTick + (g_nMaxPlayTime * 1000);
        }

        // Monitor DirectShow events until complete AND all requested
        // loops have been played.
        while (TRUE)
        {
            // If user only wants to play the first N seconds of a clip,
            // then keep track of time and exit when complete.
            if (g_bUsingMaxPlayTime)
            {
                if (GetTickCount() > ulMaxPlayTick)
                {
                    RetailOutput(TEXT("    Reached max requested play time of %d seconds.\r\n"),
                                 g_nMaxPlayTime);
                    ResetDirectShow();
                    break;
                }
            }

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

            // Did the user request to skip the current file?
            if (g_bUserSkip)
            {
                g_bUserSkip=FALSE;
                ResetDirectShow();
                RetailOutput(TEXT("Skipping current file...\r\n"));
                break;
            }

            Sleep(10);
        }

#else

        ResetDirectShow();

#endif
    }

    if (g_bShowingMainWindow)
    {
        ShowWindow(g_hwndMain, SW_HIDE);
    }


//  RetailOutput(TEXT("PlayFile: FINISHED %s\r\n"), pszFile);

    return(hr);
}



BOOL ScreenFileSize(LPTSTR pszFile)
{
    HANDLE hFile;
    LONGLONG llSize=0;
    DWORD dwSizeLow=0, dwSizeHigh=0;

    hFile = CreateFile(pszFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
                       (DWORD) 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        RetailOutput(TEXT("*** Failed(0x%x) to open file (to read size)!\r\n"),
                     GetLastError());
        return FALSE;
    }

    // Error?
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    if ((dwSizeLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    {
        RetailOutput(TEXT("*** Error(0x%x) reading file size!\r\n"),
                     GetLastError());
        return FALSE;
    }

    CloseHandle(hFile);

    // Large files will also use the upper DWORD to report size.
    // Add them together for the true size if necessary.
    if (dwSizeHigh)
        llSize = (dwSizeHigh << 16) + dwSizeLow;
    else
        llSize = dwSizeLow;

    RetailOutput(TEXT("    File size is %d bytes.\r\n"), dwSizeLow);

    // Skip bogus media files (anything 1K or smaller)
    if (llSize < ENFORCED_MIN_FILESIZE)
    {
        RetailOutput(TEXT("    SKIPPING BOGUS FILE! (smaller than %d bytes)\r\n"),
                     ENFORCED_MIN_FILESIZE);
        return TRUE;
    }

    // Is this file larger than the user requested as the upper limit?
    if (llSize > g_uMaxFileSize)
    {
        RetailOutput(TEXT("    Skipping file as requested (larger than %d bytes).\r\n"),
                     g_uMaxFileSize);
        return TRUE;
    }

    // Is this file smaller than the user requested as the lower limit?
    if (llSize < g_uMinFileSize)
    {
        RetailOutput(TEXT("    Skipping file as requested (smaller than %d bytes).\r\n"),
                     g_uMinFileSize);
        return TRUE;
    }

    return FALSE;
}


BOOL ScreenOmittedFile(LPTSTR pszFileToPlay)
{
    TCHAR szFile[MAX_PATH]={0};

    GetFilename(pszFileToPlay, szFile);

    if (!_tcscmp(szFile, g_pszOmitFile))
    {
        RetailOutput(TEXT("OMITTING requested file %s."), g_pszOmitFile);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


HRESULT PrepareMedia(LPTSTR lpszMovie, unsigned int nLoopCount)
{
    HRESULT hr = S_OK;

    SetWindowText(g_hwndMain, TEXT("PlayAll - Loading..."));

    // Allow DirectShow to create the FilterGraph for this media file
    hr = pGB->RenderFile(lpszMovie, NULL);
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in RenderFile(%s)!\r\n"),
                 hr, lpszMovie);

        // Increment global count of fails failing to render
        g_nFilesFailed++;
        return hr;
    }

    // Set the message drain of the video window to point to our hidden
    // application window.  This will allow a user to close the application
    // by pressing ESC or Q.  Otherwise, the only way to interrupt a long
    // recursive run is to kill the process - not a pleasant thing.
    //
    // If this is an audio-only or MIDI file, then put_MessageDrain will fail.
    //
    hr = pVW->put_MessageDrain((OAHWND) g_hwndMain);
    if (FAILED(hr)) {
//      RetailOutput(TEXT("\r\n*** Failed(%08lx) in put_MessageDrain(%s)!\r\n"),
//                   hr, lpszMovie);

        // Show the main hidden window for keyboard control
        ShowCurrentAudioFile(lpszMovie);

        ShowWindow(g_hwndMain, SW_SHOWNORMAL);
        g_bShowingMainWindow = TRUE;
        g_bAudioOnly = TRUE;
        SetForegroundWindow(g_hwndMain);
        SetFocus(g_hwndMain);
    }
    else
    {
        ShowCurrentVideoFile(lpszMovie);
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

    if (g_nLoopCount > 1)
    {
        RetailOutput(TEXT("    Playing [#%d]: %s (for %d iterations)\r\n"),
                     g_nFilesPlayed+1, lpszMovie, g_nLoopCount);
    }
    else
    {
        RetailOutput(TEXT("    Playing [#%d]: %s\r\n"),
                     g_nFilesPlayed+1, lpszMovie);
    }

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
        while (PeekMessage (&msg, g_hwndMain, 0, 0, PM_REMOVE))
        {
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

    if (g_bAudioOnly)
        return;

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


HRESULT PauseMedia(void)
{
    HRESULT hr=S_OK;
    static BYTE bDisabledTimer=0;

    if (!pMC)
        return S_OK;

    // Play/pause
    if((g_psCurrent == psPAUSED) || (g_psCurrent == psSTOPPED))
    {
        // If we disabled the automatic timer, reactivate it
        if (bDisabledTimer)
        {
            g_bUsingMaxPlayTime = 1;
            bDisabledTimer = 0;
        }

        hr = pMC->Run();
        if (FAILED(hr)) {
            RetailOutput(TEXT("\r\n*** Failed(%08lx) in Run()!\r\n"), hr);
            return hr;
        }
        else
            RetailOutput(TEXT("*** Media is restarting.\r\n"));

        g_psCurrent = psRUNNING;
    }
    else
    {
        // If the automatic timer is active, temporarily disable it
        if (g_bUsingMaxPlayTime)
        {
            g_bUsingMaxPlayTime = 0;
            bDisabledTimer = 1;
        }

        hr = pMC->Pause();
        if (FAILED(hr)) {
            RetailOutput(TEXT("\r\n*** Failed(%08lx) in Pause()!\r\n"), hr);
            return hr;
        }
        else
            RetailOutput(TEXT("*** Media is PAUSED.\r\n"));

        g_psCurrent = psPAUSED;
    }

    return hr;
}


HRESULT ToggleMute(void)
{
    HRESULT hr=S_OK;
    IBasicAudio *pBA=NULL;
    static long g_lVolume=VOLUME_FULL;

    if (!pGB)
        return S_OK;

    hr =  pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA);
    if (FAILED(hr) || !pBA)
        return S_OK;

    // Read current volume
    hr = pBA->get_Volume(&g_lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        pBA->Release();
        return hr;
    }
    else if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed in pBA->get_Volume!  hr=0x%x\r\n"), hr));
        pBA->Release();
        return hr;
    }

    if (g_lVolume == VOLUME_FULL)
    {
        g_lVolume = VOLUME_SILENCE;
        RetailOutput(TEXT("*** Media is MUTING.\r\n"));
    }
    else
    {
        g_lVolume = VOLUME_FULL;
        RetailOutput(TEXT("*** Media is resuming audio.\r\n"));
    }

    // Set new volume
    hr = pBA->put_Volume(g_lVolume);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed in pBA->put_Volume!  hr=0x%x\r\n"), hr));
    }

    pBA->Release();
    return hr;
}


void ShowCurrentVideoFile(LPTSTR lpszMovie)
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


void ShowCurrentAudioFile(LPTSTR lpszMovie)
{
    TCHAR szFile[MAX_PATH], szTitle[MAX_PATH];

    GetFilename(lpszMovie, szFile);
    wsprintf(szTitle, TEXT("%s - %s\0"), NAME, szFile);

    // Set the window's taskbar caption
    SetWindowText(g_hwndMain, szTitle);
}


void GetFilename(TCHAR *pszFull, TCHAR *pszFile)
{
    int nLength;
    TCHAR szPath[MAX_PATH]={0};
    BOOL bSetFilename=FALSE;

    _tcscpy(szPath, pszFull);
    nLength = _tcslen(szPath);

    for (int i=nLength-1; i>=0; i--)
    {
        if ((szPath[i] == '\\') || (szPath[i] == '/'))
        {
            szPath[i] = '\0';
            lstrcpy(pszFile, &szPath[i+1]);
            bSetFilename = TRUE;
            break;
        }
    }

    // If there was no path given (just a file name), then
    // just copy the full path to the target path.
    if (!bSetFilename)
        _tcscpy(pszFile, pszFull);
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
        // RetailOutput(TEXT("  Received unknown event code (0x%x)\r\n"), lEventCode);
        break;
    }
}


HRESULT InitDirectShow(void)
{
    HRESULT hr = S_OK;

    g_bAudioOnly = FALSE;

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
