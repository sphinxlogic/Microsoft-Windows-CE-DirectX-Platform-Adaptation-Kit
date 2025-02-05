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

#endif