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
//   Functions specific to IVideoWindow interface
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

BOOL g_bShowingMainWindow = FALSE;
LONG g_lVolume=VOLUME_FULL;


void ProcessStartupOptions(void)
{
    // Is this a video or an audio media file?
    // If audio only, don't continue processing video options.
    CheckVisibility();
    if (g_bAudioOnly)
        return;

    // Set initial window position
    SetInitialPosition();

    pVW->put_Visible(OATRUE);
    pVW->SetWindowForeground(-1);

    if (g_bCenterStart)
        CenterVideo();

    if (g_bQuadrantStart)
        SetVideoQuadrant(g_wInitialQuadrant-1);
}


void CheckVisibility(void)
{
    long lVisible;
    HRESULT hr;

    hr = pVW->get_Visible(&lVisible);
    if (FAILED(hr))
    {
        // If this is an audio-only clip, get_Visible() won't work.
        //
        // Also, if this video is encoded with an unsupported codec,
        // we won't see any video, although the audio will work if it is
        // of a supported format.
        //
        if (hr == E_NOINTERFACE)
        {
            g_bAudioOnly = TRUE;
            OutputDebugString(TEXT("Video window not visible.  Assuming audio file or unsupported video codec.\r\n"));
        }
        else
        {
            RETAILMSG(1, (TEXT("Failed(%08lx) in pVW->get_Visible()!\r\n"), hr));
        }

        return;
    }

    if (!lVisible)
    {
        g_bAudioOnly = TRUE;
        OutputDebugString(TEXT("Media file has no video content.  No visible window.\r\n"));
    }
    else
    {
        pVW->SetWindowForeground(-1);
        SetFocus(g_hwndMain);
    }
}


void SetInitialPosition(void)
{
    HRESULT hr;
    long left, top, width, height;

    // First read the current size and position
    hr = pVW->GetWindowPosition(&left, &top, &width, &height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to read initial window position!  hr=0x%x\r\n"), hr));
        return;
    }

    // If no command line parameters specified, go with these system defaults.
    // Otherwise, honor the command line parameters.
    if (!leftStart)
        leftStart = left;
    if (!topStart)
        topStart = top;
    if (!widthStart)
        widthStart = width;
    if (!heightStart)
        heightStart = height;

    // Set original video coordinates, which may be manipulated by
    // command line parameters (-x, -y, -w, -h).
    hr = pVW->SetWindowPosition(leftStart, topStart, widthStart, heightStart);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to set initial window position!  hr=0x%x\r\n"), hr));
        return;
    }

//  RETAILMSG(1, (TEXT("Initialized video at X=%d Y=%d  Width=%d Height=%d\r\n"),
//            leftStart, topStart, widthStart, heightStart));
}


void SetVideoQuadrant(WORD wQuad)
{
    HRESULT hr;

    if (wQuad >= NUM_QUADRANTS_SET)
    {
        RETAILMSG(1, (TEXT("Invalid quadrant setting %d!\r\n"), wQuad));
        return;
    }

    PQUAD pq = &quadrants[wQuad];

    hr = pVW->SetWindowPosition(pq->left, pq->top, pq->width, pq->height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to change window position!  hr=0x%x\r\n"), hr));
        return;
    }

//  RETAILMSG(1, (TEXT("Set video to quadrant %d [%d x %d at (%d,%d)].\r\n"),
//           (wQuad + 1), pq->width, pq->height, pq->left, pq->top ));
}


void ModifyVideoSize(BOOL bUp)
{
    HRESULT hr;
    long left, top, width, height;

    hr = pVW->GetWindowPosition(&left, &top, &width, &height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to read window position!  hr=0x%x\r\n"), hr));
        return;
    }

    if (bUp == UP)
    {
        width  += kWidthAdjust;
        height += kHeightAdjust;

        if (width > kMaxWidth)
            width = kMaxWidth;
        if (height > kMaxHeight)
            height = kMaxHeight;
    }
    else
    {
        width  -= kWidthAdjust;
        height -= kHeightAdjust;

        if (width < kMinWidth)
            width = kMinWidth;
        if (height < kMinHeight)
            height = kMinHeight;
    }

    hr = pVW->SetWindowPosition(left, top, width, height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to change window position!  hr=0x%x\r\n"), hr));
        return;
    }
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
        RETAILMSG(1, (TEXT("Failed to set window position!  hr=0x%x\r\n"), hr));
        return;
    }

//  RETAILMSG(1, (TEXT("Centered video at X=%d Y=%d  Width=%d Height=%d\r\n"),
//              nCX, nCY, width, height));
}


void ResetVideoMinimum(void)
{
    LONGLONG pos = 0;
    LONG minWidth=0, minHeight=0;
    LONG posLeft,posTop,posWidth,posHeight;
    HRESULT hr;

    hr = pVW->GetMinIdealImageSize(&minWidth, &minHeight);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to read Min ideal size!  hr=0x%x\r\n"), hr));
        return;
    }

//  RETAILMSG(1, (TEXT("Setting ideal minimum coordinates of X=%d Y=%d  Width=%d Height=%d\r\n"),
//              0, 0, width, height));

    // If we're currently toggled to 'video maximum', restore to previous
    // window position and size
    if (g_vsCurrent == vsFULLSCREEN)
        ToggleVideoMaximum();

    // Read the current window position and size
    hr = pVW->GetWindowPosition(&posLeft, &posTop, &posWidth, &posHeight);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to read window position!  hr=0x%x\r\n"), hr));
        return;
    }

    // Set to the minimum recommended width and height
    hr = pVW->SetWindowPosition(posLeft, posTop, minWidth, minHeight);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to set window position!  hr=0x%x\r\n"), hr));
        return;
    }

    hr = pMC->Run();
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to Run filter graph!  hr=0x%x\r\n"), hr));
        return;
    }
    g_psCurrent = psRUNNING;
}


void ToggleVideoMaximum(void)
{
    // toggle full-screen and default size
    if (g_vsCurrent == vsDEFAULT)
    {
//      RETAILMSG(1, (TEXT("CEPlayit: Setting to max screen (X=%d  Y=%d)\r\n"),
//                  g_nMaxX, g_nMaxY));

        pVW->GetWindowPosition(&leftSave, &topSave, &widthSave, &heightSave);
        pVW->SetWindowPosition(0, 0, g_nMaxX, g_nMaxY);
        g_vsCurrent = vsFULLSCREEN;
    }
    else
    {
        pVW->SetWindowPosition(leftSave, topSave, widthSave, heightSave);
        g_vsCurrent = vsDEFAULT;
    }
}


void ResetVideo(void)
{
    LONGLONG pos = 0;
    HRESULT hr;

    OutputDebugString(TEXT("CEPlayit: Replaying video...\r\n"));

    pMC->Stop();
    pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                        NULL, AM_SEEKING_NoPositioning);
    g_psCurrent = psSTOPPED;

    RETAILMSG(1, (TEXT("Restoring original coordinates of X=%d Y=%d  Width=%d Height=%d\r\n"),
                leftStart, topStart, widthStart, heightStart));

    // Restore original video coordinates
    hr = pVW->SetWindowPosition(leftStart, topStart, widthStart, heightStart);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to set window position!  hr=0x%x\r\n"), hr));
        return;
    }

    pMC->Run();
    g_psCurrent = psRUNNING;
}


void ResetAudio(void)
{
    LONGLONG pos = 0;
    HRESULT hr;

    OutputDebugString(TEXT("CEPlayit: Replaying audio...\r\n"));

    pMC->Stop();
    pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                        NULL, AM_SEEKING_NoPositioning);
    g_psCurrent = psSTOPPED;

    hr = pMC->Run();
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to set Run state for audio!  hr=0x%x\r\n"), hr));
        return;
    }
    g_psCurrent = psRUNNING;
}



HRESULT ProcessMoveKeys(WPARAM wParam)
{
    HRESULT hr;
    long left, top, width, height;

    hr = pVW->GetWindowPosition(&left, &top, &width, &height);

    // If SHIFT is pressed, stretch the window
    if (GetKeyState(VK_SHIFT) & ((SHORT) 0x8000))
    {
        switch (wParam)
        {
            case VK_UP:
                if (height - kSizeAdjust >= kMinHeight)
                    hr = pVW->put_Height(height - kSizeAdjust);
                break;
            case VK_DOWN:
                if (height + kSizeAdjust <= g_nMaxY)
                    hr = pVW->put_Height(height + kSizeAdjust);
                break;
            case VK_LEFT:
                if (width - kSizeAdjust >= kMinWidth)
                    hr = pVW->put_Width(width - kSizeAdjust);
                break;
            case VK_RIGHT:
                if (width + kSizeAdjust <= g_nMaxX)
                    hr = pVW->put_Width(width + kSizeAdjust);
                break;
        }
    }

    // If bare key, move window position
    else if (GetKeyState(VK_SHIFT) == 0)
    {
        switch (wParam)
        {
            case VK_UP:
                if (top - kMoveAdjust >= 0)
                    hr = pVW->put_Top(top - kMoveAdjust);
                break;
            case VK_DOWN:
                if (top + kMoveAdjust <= g_nMaxY)
                    hr = pVW->put_Top(top + kMoveAdjust);
                break;
            case VK_LEFT:
                if (left - kMoveAdjust >= 0)
                    hr = pVW->put_Left(left - kMoveAdjust);
                break;
            case VK_RIGHT:
                if (left + kMoveAdjust <= g_nMaxX)
                    hr = pVW->put_Left(left + kMoveAdjust);
                break;
        }
    }

    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("ProcessMoveKey: Operation failed!  hr=0x%x\r\n"), hr));
    }
    return hr;
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


void ShowCurrentVideoFile(LPTSTR lpszMovie)
{
    TCHAR szFile[MAX_PATH], szTitle[MAX_PATH];
    BSTR bstrFile;
    HRESULT hr;

    GetFilename(lpszMovie, szFile);
    wsprintf(szTitle, TEXT("%s - %s\0"), NAME, szFile);

    // Set the window's taskbar caption
    bstrFile = SysAllocString(szTitle);
    hr = pVW->put_Caption(bstrFile);
    if (FAILED(hr))
        RETAILMSG(1, (TEXT("ShowCurrentVideoFile: pVW->put_Caption failed!  hr=0x%x\r\n"), hr));

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


void PauseClip(void)
{
    if (!pMC)
        return;

    // Play/pause
    if((g_psCurrent == psPAUSED) || (g_psCurrent == psSTOPPED))
    {
        pMC->Run();
        g_psCurrent = psRUNNING;
    }
    else
    {
        pMC->Pause();
        g_psCurrent = psPAUSED;
    }
}


void StopClip(void)
{
    if ((!pMC) || (!pMS))
        return;

    // Stop and reset postion to beginning
    if((g_psCurrent == psPAUSED) || (g_psCurrent == psRUNNING))
    {
        LONGLONG pos = 0;
        pMC->Stop();
        g_psCurrent = psSTOPPED;

        pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
            NULL, AM_SEEKING_NoPositioning);

        // Display the first frame to indicate the reset condition
        pMC->Pause();
    }
}


HRESULT ToggleMute(void)
{
    HRESULT hr;
    IBasicAudio *pBA=NULL;

    if (!pGB)
        return S_OK;

    // Get audio interface pointer to control volume
    hr = pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA);

    if (FAILED(hr) || (!pBA))
        return hr;

    // Read current volume
    hr = pBA->get_Volume(&g_lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        return hr;
    }
    else if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed in pBA->get_Volume!  hr=0x%x\r\n"), hr));
        return hr;
    }

    if (g_lVolume == VOLUME_FULL)
    {
        g_lVolume = VOLUME_SILENCE;
        RETAILMSG(1, (TEXT("Audio has been muted.\r\n")));
    }
    else
    {
        g_lVolume = VOLUME_FULL;
        RETAILMSG(1, (TEXT("Audio has been returned to normal volume.\r\n")));
    }

    // Set new volume
    pBA->put_Volume(g_lVolume);
    pBA->Release();

    return hr;
}


HRESULT ToggleFullScreen(void)
{
    HRESULT hr=S_OK;
    LONG lMode;
    static HWND hDrain=0;
    BOOL bPausedForTransition=FALSE;

    // Don't bother with full-screen for audio-only files
    if (g_bAudioOnly)
        return S_OK;

    if (!pVW)
        return S_OK;

    // If we're currently playing a media file, pause temporarily
    // to prevent audio/video breakup during/after the transition.
    if (g_psCurrent == psRUNNING)
    {
        PauseClip();
        bPausedForTransition = TRUE;
    }

    // Read current state
    JIF(pVW->get_FullScreenMode(&lMode));

    if (lMode == OAFALSE)
    {
        // Save current message drain
        JIF(pVW->get_MessageDrain((OAHWND *) &hDrain));

        // Set message drain to application main window
        JIF(pVW->put_MessageDrain((OAHWND) g_hwndMain));

        // Switch to full-screen mode
        lMode = OATRUE;
        JIF(pVW->put_FullScreenMode(lMode));

        JIF(pVW->get_FullScreenMode(&lMode));
        if (lMode != OATRUE)
            OutputDebugString(TEXT("Error: Didn't set fullscreen mode!\r\n"));
    }
    else
    {
        // Switch back to windowed mode
        lMode = OAFALSE;
        JIF(pVW->put_FullScreenMode(lMode));

        // Undo change of message drain
        LIF(pVW->put_MessageDrain((OAHWND) hDrain));

        // Reset video window
        LIF(pVW->SetWindowForeground(-1));

        // Reclaim keyboard focus for player application
        UpdateWindow(g_hwndMain);
        SetForegroundWindow(g_hwndMain);
        SetFocus(g_hwndMain);
    }

CLEANUP:
    // Restore playback state if we previously paused
    if (bPausedForTransition)
    {
        PauseClip();
    }

    return hr;
}


