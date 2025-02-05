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

    if (g_bFullScreenStart)
        ToggleFullscreen();
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

    RETAILMSG(1, (TEXT("Initialized video at X=%d Y=%d  Width=%d Height=%d\r\n"),
              leftStart, topStart, widthStart, heightStart));
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

    CenterVideo();
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
    LONG width=0, height=0;
    HRESULT hr;

    hr = pVW->GetMinIdealImageSize(&width, &height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to read Min ideal size!  hr=0x%x\r\n"), hr));
        return;
    }

    RETAILMSG(1, (TEXT("Setting ideal minimum coordinates of X=%d Y=%d  Width=%d Height=%d\r\n"),
                0, 0, width, height));

    hr = pVW->SetWindowPosition(0, 0, width, height);
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


void ResetVideoMaximum(void)
{
    LONGLONG pos = 0;
    LONG width=0, height=0;
    HRESULT hr;

    hr = pVW->GetMaxIdealImageSize(&width, &height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to read Max ideal size!  hr=0x%x\r\n"), hr));
        return;
    }

    RETAILMSG(1, (TEXT("Setting ideal maximum coordinates of X=%d Y=%d  Width=%d Height=%d\r\n"),
                0, 0, width, height));

    hr = pVW->SetWindowPosition(0, 0, width, height);
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to set window position!  hr=0x%x\r\n"), hr));
        return;
    }

    pMC->Run();
    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed to Run filter graph!  hr=0x%x\r\n"), hr));
        return;
    }
    g_psCurrent = psRUNNING;
}


void CheckVisibility(void)
{
    TCHAR buf[256];
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
            wsprintf(buf, TEXT("Failed(%08lx) in pVW->get_Visible()!\r\n"), hr);
            OutputDebugString(buf);
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



void ToggleFullscreen(void)
{
    // toggle full-screen and default size
    if (g_vsCurrent == vsDEFAULT)
    {
        RETAILMSG(1, (TEXT("CEPlayit: Setting to FullScreen (X=%d  Y=%d)\r\n"),
                    g_nMaxX, g_nMaxY));

        pVW->GetWindowPosition(&leftSave, &topSave, &widthSave, &heightSave);
        pVW->SetWindowPosition(0, 0, g_nMaxX, g_nMaxY);

//      RETAILMSG(1, (TEXT("Saving coordinates as X=%d Y=%d  Width=%d Height=%d\r\n"),
//                leftSave, topSave, widthSave, heightSave));
        g_vsCurrent = vsFULLSCREEN;
    }
    else
    {
//      RETAILMSG(1, (TEXT("CEPlayit: Setting to previous (or default) window size\r\n")));

        RETAILMSG(1, (TEXT("Restoring coordinates to X=%d Y=%d  Width=%d Height=%d\r\n"),
                  leftSave, topSave, widthSave, heightSave));

        pVW->SetWindowPosition(leftSave, topSave, widthSave, heightSave);

        g_vsCurrent = vsDEFAULT;
    }
}


HRESULT ProcessMoveKeysVW(WPARAM wParam)
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

