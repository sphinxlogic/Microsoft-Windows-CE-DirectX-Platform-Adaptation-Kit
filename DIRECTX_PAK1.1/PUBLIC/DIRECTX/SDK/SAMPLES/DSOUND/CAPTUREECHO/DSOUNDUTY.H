
//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 2000 Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;

#ifndef __DSOUNDUTY_H__
#define __DSOUNDUTY_H__

// External dependencies
#include "map.h"
#include "ComUty.h"

DECLARE_STD_UUIDOF_SPECIALIZATION(IDirectSound);
DECLARE_STD_UUIDOF_SPECIALIZATION(IDirectSoundCapture);
DECLARE_STD_UUIDOF_SPECIALIZATION(IDirectSoundBuffer);
DECLARE_STD_UUIDOF_SPECIALIZATION(IDirectSoundCaptureBuffer);
DECLARE_STD_UUIDOF_SPECIALIZATION(IDirectSoundNotify);

extern const static_map<HRESULT, LPCTSTR> g_ErrorMap;

extern HRESULT ClearSoundBuffer(IN IDirectSoundBuffer* piDSB);

extern HRESULT GetDefaultCaptureFormat(IN IDirectSoundCapture* piDSC, OUT WAVEFORMATEX& wfx);
extern HRESULT SetRequestedCaptureFormat(IN IDirectSoundCapture* piDSC, OUT WAVEFORMATEX& wfx,
                                  IN DWORD dwFormatIndex);

#endif