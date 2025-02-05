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
//  Global data for PlayAll DirectShow sample
//

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include <control.h>
#include <stdio.h>
#include <vfwmsgs.h>
#include <evcode.h>

#include "playall.h"

//
// Global data
//
IGraphBuilder *pGB = NULL;
IMediaSeeking *pMS = NULL;
IMediaControl *pMC = NULL;
IMediaEvent   *pME = NULL;
IBasicVideo   *pBV = NULL;
IVideoWindow  *pVW = NULL;

PLAYSTATE g_psCurrent=psSTOPPED;
VIDEOSIZE g_vsCurrent=vsDEFAULT;

HWND g_hwndMain=0;                        // Window to receive messages
BOOL g_bContinue=TRUE,
     g_bLooping=FALSE,
     g_bAudioOnly=FALSE,
     g_bDisplayEvents=FALSE,
     g_bFullScreen=TRUE;                  // Fullscreen by default

int  g_nFilesPlayed=0, g_nFilesFailed=0, g_nAudioOnly=0, g_nGlobalRepeats=1;

int  g_nMaxX=640, g_nMaxY=480;

LONG leftSave=0, topSave=0, widthSave=0, heightSave=0;
LONG leftStart=0, topStart=0, widthStart=0, heightStart=0;
BOOL g_bFullScreenStart=0, g_bCenterStart=0;

