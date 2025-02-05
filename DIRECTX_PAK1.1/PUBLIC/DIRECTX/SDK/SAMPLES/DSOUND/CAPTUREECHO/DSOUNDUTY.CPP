
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

#include "main.h"
#include "ComUty.h"
#include "DebugUty.h"
#include "DSoundUty.h"


#define ARRAYSIZE(A) (sizeof(A)/sizeof(A[0]))
#define BEGIN(A) &(A[0])
#define END(A) &(A[ARRAYSIZE(A)])
#define VECTOR(A) BEGIN(A),END(A)

// g_ErrorMap
// ==========
namespace
{
#   define VALUE_TYPE static_map<HRESULT, LPCTSTR>::value_type
#   define ENTRY(X) VALUE_TYPE(X, TEXT(#X))
    const VALUE_TYPE l_rgDSErrorMap[] =
    {
        ENTRY(DS_OK),
        ENTRY(DSERR_ALLOCATED),
        ENTRY(DSERR_CONTROLUNAVAIL),
        ENTRY(DSERR_INVALIDPARAM),
        ENTRY(DSERR_INVALIDCALL),
        ENTRY(DSERR_GENERIC),
        ENTRY(DSERR_PRIOLEVELNEEDED),
        ENTRY(DSERR_OUTOFMEMORY),
        ENTRY(DSERR_BADFORMAT),
        ENTRY(DSERR_UNSUPPORTED),
        ENTRY(DSERR_NODRIVER),
        ENTRY(DSERR_ALREADYINITIALIZED),
        ENTRY(DSERR_NOAGGREGATION),
        ENTRY(DSERR_BUFFERLOST),
        ENTRY(DSERR_OTHERAPPHASPRIO),
        ENTRY(DSERR_UNINITIALIZED),
        ENTRY(DSERR_NOINTERFACE),
#       if CFG_PLATFORM_DREAMCAST
        ENTRY(DSERR_NOT32BYTEALIGNED),
#       endif
    };
#   undef ENTRY
#   undef VALUE_TYPE
}

const static_map<HRESULT, LPCTSTR> g_ErrorMap(VECTOR(l_rgDSErrorMap));

// ClearSoundBuffer
// ================
HRESULT ClearSoundBuffer(IN IDirectSoundBuffer* piDSB)
{
    HRESULT hr;
    LPBYTE rpbAudioPlaybackPtr[2];
    DWORD rdwAudioPlaybackBytes[2];

    // Make sure the buffer has stopped
    hr = piDSB->Stop();
    CheckHRESULT(hr,"IDirectSoundBuffer::Stop", return hr);

    // Lock down the whole buffer
    hr = piDSB->Lock(0, 0,
                     (LPVOID*)&rpbAudioPlaybackPtr[0], &rdwAudioPlaybackBytes[0],
                     (LPVOID*)&rpbAudioPlaybackPtr[1], &rdwAudioPlaybackBytes[1],
                     DSBLOCK_ENTIREBUFFER );
    CheckHRESULT(hr, "IDirectSoundBuffer::Lock", return hr);

    // clear the bits
    if (rdwAudioPlaybackBytes[0])
        memset(rpbAudioPlaybackPtr[0], 0, rdwAudioPlaybackBytes[0]);

    if (rdwAudioPlaybackBytes[1])
        // generally speaking this is not expected to happen, however it
        // is not a garauntee so this ensures that the condition is properly handled.
        memset(rpbAudioPlaybackPtr[1], 0, rdwAudioPlaybackBytes[1]);

    // Unlock
    hr = piDSB->Unlock((LPVOID*)rpbAudioPlaybackPtr[0], rdwAudioPlaybackBytes[0],
                       (LPVOID*)rpbAudioPlaybackPtr[1], rdwAudioPlaybackBytes[1]);
    CheckHRESULT(hr, "IDirectSoundBuffer::Unlock", return hr);

    return S_OK;
}


HRESULT GetDefaultCaptureFormat(IN IDirectSoundCapture* piDSC, OUT WAVEFORMATEX& wfx)
{
    HRESULT hr;
    DSCCAPS dscc = { sizeof(DSCCAPS) };

    hr = piDSC->GetCaps(&dscc);
    CheckHRESULT(hr, "IDirectSoundCapture::GetCaps", return hr);

    if (dscc.dwFormats&WAVE_FORMAT_4S16) //  44.1 kHz, stereo, 16-bit
    {
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_4S08) //  44.1 kHz, stereo, 8-bit
    {
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 8;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_4M16) //  44.1 kHz, mono, 16-bit
    {
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_4M08) //  44.1 kHz, mono, 8-bit
    {
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 8;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_2S16) //  22.05 kHz, stereo, 16-bit
    {
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 22050;
        wfx.wBitsPerSample = 16;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_2S08) //  22.05 kHz, stereo, 8-bit
    {
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 22050;
        wfx.wBitsPerSample = 8;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_2M16) //  22.05 kHz, mono, 16-bit
    {
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 22050;
        wfx.wBitsPerSample = 16;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_2M08) //  22.05 kHz, mono, 8-bit
    {
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 22050;
        wfx.wBitsPerSample = 8;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_1S16) //  11.025 kHz, stereo, 16-bit
    {
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 11025;
        wfx.wBitsPerSample = 16;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_1S08) //  11.025 kHz, stereo, 8-bit
    {
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 11025;
        wfx.wBitsPerSample = 8;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_1M16) //  11.025 kHz, mono, 16-bit
    {
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 11025;
        wfx.wBitsPerSample = 16;
    }
    else if (dscc.dwFormats&WAVE_FORMAT_1M08) //  11.025 kHz, mono, 8-bit
    {
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = 11025;
        wfx.wBitsPerSample = 8;
    }
    else
    {
        dbgout(TEXT("ERROR: IDirectSoundCapture::GetCaps returned invalid DSCCAPS.dwFormats\n"));
        return DSERR_UNSUPPORTED;
    }

    wfx.wFormatTag=WAVE_FORMAT_PCM;
    wfx.nBlockAlign = (wfx.wBitsPerSample*wfx.nChannels)/8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign*wfx.nSamplesPerSec;
    wfx.cbSize = 0;

    return hr;
}



HRESULT SetRequestedCaptureFormat(IN IDirectSoundCapture* piDSC, OUT WAVEFORMATEX& wfx,
                                  IN DWORD dwFormatIndex)
{
    HRESULT hr;
    DSCCAPS dscc = { sizeof(DSCCAPS) };

    hr = piDSC->GetCaps(&dscc);
    CheckHRESULT(hr, "IDirectSoundCapture::GetCaps", return hr);

    // Is the requested format supported?
    if (! (dscc.dwFormats & (1 << dwFormatIndex)))
    {
        dbgout(TEXT("This format is not supported by the audio driver!\n"));
        return DSERR_GENERIC;
    }

    typedef struct _wfxfill
    {
        int  nChannels;
        int  nSamplesPerSec;
        WORD wBitsPerSample;
        TCHAR szDescription[24];

    } WFXFILL;

    WFXFILL wfxfill[NUM_SUPPORTED_CAPTURE_FORMATS] = {
        1, 11025, 8,  TEXT("11kHz 8-bit mono"),
        1, 11025, 16, TEXT("11kHz 8-bit stereo"),
        2, 11025, 8,  TEXT("11kHz 16-bit mono"),
        2, 11025, 16, TEXT("11kHz 16-bit stereo"),
        1, 22050, 8,  TEXT("22kHz 8-bit mono"),
        1, 22050, 16, TEXT("22kHz 8-bit stereo"),
        2, 22050, 8,  TEXT("22kHz 16-bit mono"),
        2, 22050, 16, TEXT("22kHz 16-bit stereo"),
        1, 44100, 8,  TEXT("44kHz 8-bit mono"),
        1, 44100, 16, TEXT("44kHz 8-bit stereo"),
        2, 44100, 8,  TEXT("44kHz 16-bit mono"),
        2, 44100, 16, TEXT("44kHz 16-bit stereo")
    };

    // Fill WFX structure with format-specific information
    wfx.nChannels      = wfxfill[dwFormatIndex].nChannels;
    wfx.nSamplesPerSec = wfxfill[dwFormatIndex].nSamplesPerSec;
    wfx.wBitsPerSample = wfxfill[dwFormatIndex].wBitsPerSample;

    // Fill remaining fields with generic or calculated values
    wfx.wFormatTag     = WAVE_FORMAT_PCM;
    wfx.nBlockAlign    = (wfx.wBitsPerSample*wfx.nChannels)/8;
    wfx.nAvgBytesPerSec= wfx.nBlockAlign*wfx.nSamplesPerSec;
    wfx.cbSize         = 0;

    dbgout(TEXT("Set capture format to %s.\n"), wfxfill[dwFormatIndex].szDescription);
    return hr;
}

