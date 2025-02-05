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
//  Global data for CEPlayit
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

//
// Global data
//
IGraphBuilder  *pGB = NULL;
IMediaControl  *pMC = NULL;
IMediaSeeking  *pMS = NULL;
IMediaPosition *pMP = NULL;
IVideoWindow   *pVW = NULL;
IBasicVideo    *pBV = NULL;
IMediaEvent    *pME = NULL;
IQualProp      *pQP = NULL;
IBaseFilter    *pBF_OvM = NULL;

int  g_nMaxX=640, g_nMaxY=480;

PLAYSTATE g_psCurrent=psSTOPPED;
VIDEOSIZE g_vsCurrent=vsDEFAULT;

HWND g_hwndMain=0;                        // Window to receive messages
BOOL g_bContinue=TRUE, g_bLooping=FALSE, g_bAudioOnly=FALSE;
BOOL g_bShowEvents=FALSE, g_bWindowless=FALSE;
BOOL g_bFullScreenStart=0, g_bCenterStart=0, g_bQuadrantStart=0;

LONG leftSave=0,  topSave=0,  widthSave=0,  heightSave=0;
LONG leftStart=0, topStart=0, widthStart=0, heightStart=0;

WORD g_wInitialQuadrant=0;

QUAD quadrants[NUM_QUADRANTS_SET] = {
  0,0,320,240,  320,0,320,240,  0,240,320,240,  320,240,320,240,
  0,0,320,480,  320,0,320,480,  0,0,640,240,    0,240,640,240,  140,0,500,480};

