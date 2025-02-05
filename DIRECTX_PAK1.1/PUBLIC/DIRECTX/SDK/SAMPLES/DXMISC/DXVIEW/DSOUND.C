/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: dsound.c

    PURPOSE: DirectX Device Viewer - DirectSound module

****************************************************************************/

#include <stdio.h>
#include "dxview.h"
#include "support.h"
#include "dsinfo.h"

//
//  Global data
//
IDirectSound *lpDS;                 // DirectSound object
GUID *       dsid;

IDirectSoundCapture *lpDSC;         // DirectSoundCapture object
GUID *       dscid;

HINSTANCE g_hDSoundLib = NULL;


/****************************************************************************
 ***************************************************************************/
CAPDEF DSInfo[] =
{
    DSVALDEF(TEXT("MinSecondarySampleRate"),          dwMinSecondarySampleRate),
    DSVALDEF(TEXT("MaxSecondarySampleRate"),          dwMaxSecondarySampleRate),
    DSVALDEF(TEXT("PrimaryBuffers"),                  dwPrimaryBuffers),
    DSVALDEF(TEXT("MaxHwMixingAllBuffers"),           dwMaxHwMixingAllBuffers),
    DSVALDEF(TEXT("MaxHwMixingStaticBuffers"),        dwMaxHwMixingStaticBuffers),
    DSVALDEF(TEXT("MaxHwMixingStreamingBuffers"),     dwMaxHwMixingStreamingBuffers),
    DSVALDEF(TEXT("FreeHwMixingAllBuffers"),          dwFreeHwMixingAllBuffers),
    DSVALDEF(TEXT("FreeHwMixingStaticBuffers"),       dwFreeHwMixingStaticBuffers),
    DSVALDEF(TEXT("FreeHwMixingStreamingBuffers"),    dwFreeHwMixingStreamingBuffers),
    DSVALDEF(TEXT("MaxHw3DAllBuffers"),               dwMaxHw3DAllBuffers),
    DSVALDEF(TEXT("MaxHw3DStaticBuffers"),            dwMaxHw3DStaticBuffers),
    DSVALDEF(TEXT("MaxHw3DStreamingBuffers"),         dwMaxHw3DStreamingBuffers),
    DSVALDEF(TEXT("FreeHw3DAllBuffers"),              dwFreeHw3DAllBuffers),
    DSVALDEF(TEXT("FreeHw3DStaticBuffers"),           dwFreeHw3DStaticBuffers),
    DSVALDEF(TEXT("FreeHw3DStreamingBuffers"),        dwFreeHw3DStreamingBuffers),
    DSVALDEF(TEXT("TotalHwMemBytes"),                 dwTotalHwMemBytes),
    DSVALDEF(TEXT("FreeHwMemBytes"),                  dwFreeHwMemBytes),
    DSVALDEF(TEXT("MaxContigFreeHwMemBytes"),         dwMaxContigFreeHwMemBytes),
    DSVALDEF(TEXT("UnlockTransferRateHwBuffers"),     dwUnlockTransferRateHwBuffers),
    DSVALDEF(TEXT("PlayCpuOverheadSwBuffers"),        dwPlayCpuOverheadSwBuffers),
    {TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF DSGeneralCaps[] =
{
    DSCAPDEF(TEXT("PRIMARYMONO"),       dwFlags,    DSCAPS_PRIMARYMONO),
    DSCAPDEF(TEXT("PRIMARYSTEREO"),     dwFlags,    DSCAPS_PRIMARYSTEREO),
    DSCAPDEF(TEXT("PRIMARY8BIT"),       dwFlags,    DSCAPS_PRIMARY8BIT),
    DSCAPDEF(TEXT("PRIMARY16BIT"),      dwFlags,    DSCAPS_PRIMARY16BIT),
    DSCAPDEF(TEXT("CONTINUOUSRATE"),    dwFlags,    DSCAPS_CONTINUOUSRATE),
    DSCAPDEF(TEXT("EMULDRIVER"),        dwFlags,    DSCAPS_EMULDRIVER),
    DSCAPDEF(TEXT("SECONDARYMONO"),     dwFlags,    DSCAPS_SECONDARYMONO),
    DSCAPDEF(TEXT("SECONDARYSTEREO"),   dwFlags,    DSCAPS_SECONDARYSTEREO),
    DSCAPDEF(TEXT("SECONDARY8BIT"),     dwFlags,    DSCAPS_SECONDARY8BIT),
    DSCAPDEF(TEXT("SECONDARY16BIT"),    dwFlags,    DSCAPS_SECONDARY16BIT),
    {TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEFS DSCapDefs[] =
{
    {TEXT(""),                    DSAddCaps,          (LPARAM)DSInfo,         DSPrintCaps},
    {TEXT("General"),             DSAddCaps,          (LPARAM)DSInfo,         DSPrintCaps},
    {TEXT("General Caps"),        DSAddCaps,          (LPARAM)DSGeneralCaps,  DSPrintCaps},
    {NULL, 0, 0, NULL}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF DSCInfo[] =
{
    DSCVALDEF(TEXT("Channels"),          dwChannels),
    {TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF DSCGeneralCaps[] =
{
    DSCCAPDEF(TEXT("EMULDRIVER"),        dwFlags,    DSCCAPS_EMULDRIVER),
    DSCCAPDEF(TEXT("WAVE_FORMAT_1M08"),  dwFormats,  WAVE_FORMAT_1M08),
    DSCCAPDEF(TEXT("WAVE_FORMAT_1S08"),  dwFormats,  WAVE_FORMAT_1S08),
    DSCCAPDEF(TEXT("WAVE_FORMAT_1M16"),  dwFormats,  WAVE_FORMAT_1M16),
    DSCCAPDEF(TEXT("WAVE_FORMAT_1S16"),  dwFormats,  WAVE_FORMAT_1S16),
    DSCCAPDEF(TEXT("WAVE_FORMAT_2M08"),  dwFormats,  WAVE_FORMAT_2M08),
    DSCCAPDEF(TEXT("WAVE_FORMAT_2S08"),  dwFormats,  WAVE_FORMAT_2S08),
    DSCCAPDEF(TEXT("WAVE_FORMAT_2M16"),  dwFormats,  WAVE_FORMAT_2M16),
    DSCCAPDEF(TEXT("WAVE_FORMAT_2S16"),  dwFormats,  WAVE_FORMAT_2S16),
    DSCCAPDEF(TEXT("WAVE_FORMAT_4M08"),  dwFormats,  WAVE_FORMAT_4M08),
    DSCCAPDEF(TEXT("WAVE_FORMAT_4S08"),  dwFormats,  WAVE_FORMAT_4S08),
    DSCCAPDEF(TEXT("WAVE_FORMAT_4M16"),  dwFormats,  WAVE_FORMAT_4M16),
    DSCCAPDEF(TEXT("WAVE_FORMAT_4S16"),  dwFormats,  WAVE_FORMAT_4S16),
    {TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEFS DSCCapDefs[] =
{
    {TEXT(""),                    DSCAddCaps,          (LPARAM)DSCInfo,         DSCPrintCaps},
    {TEXT("General"),             DSCAddCaps,          (LPARAM)DSCInfo,         DSCPrintCaps},
    {TEXT("General Caps"),        DSCAddCaps,          (LPARAM)DSCGeneralCaps,  DSCPrintCaps},
    {NULL, 0, 0, NULL}
};


//================================================================
//================================================================
IDirectSound * DSCreate(GUID *pid)
{
    PFN_DSCREATE pfn_DSCreate;

    if (lpDS && pid == dsid)
        return lpDS;

    if (lpDS)
    {
        IDirectSound_Release(lpDS);
        lpDS = NULL;
    }

    // Call DirectSoundCreate.
    pfn_DSCreate = (PFN_DSCREATE)GetProcAddress(g_hDSoundLib, TEXT("DirectSoundCreate"));
    if (pfn_DSCreate == NULL)
    {
        MessageBox(g_hwndMain, TEXT("DSOUND.DLL does not implement DirectSoundCreate()"), g_szAppName, MB_OK);
        return NULL;
    }

    if ((*pfn_DSCreate)(pid, &lpDS, NULL) == DD_OK)
    {
        dsid = pid;
        return lpDS;
    }

    if(!g_PrintToFile)
        MessageBox(g_hwndMain, TEXT("DirectSoundCreate failed."), g_szAppName, MB_OK);
    return NULL;
}


//================================================================
//================================================================
IDirectSoundCapture * DSCCreate(GUID *pid)
{
    PFN_DSCCREATE pfn_DSCCreate;

    if (lpDSC && pid == dsid)
        return lpDSC;

    if (lpDSC)
    {
        IDirectSoundCapture_Release(lpDSC);
        lpDSC = NULL;
    }

    pfn_DSCCreate = (PFN_DSCCREATE)GetProcAddress(g_hDSoundLib, TEXT("DirectSoundCaptureCreate"));
    if (pfn_DSCCreate == NULL)
    {
        MessageBox(g_hwndMain, TEXT("DSOUND.DLL does not implement DirectSoundCaptureCreate()"), g_szAppName, MB_OK);
        return NULL;
    }

    if ((*pfn_DSCCreate)(pid, &lpDSC, NULL) == DD_OK)
    {
        dscid = pid;
        return lpDSC;
    }

    if(!g_PrintToFile)
        MessageBox(g_hwndMain, TEXT("DirectSoundCaptureCreate failed."), g_szAppName, MB_OK);
    return NULL;
}


//================================================================
//================================================================
void DSAddCaps(LPARAM lParam1, LPARAM lParam2)
{
    // lParam1 is the GUID for the driver we should open
    // lParam2 is the CAPDEF table we should use

    if (DSCreate((GUID*)lParam1))
    {
        DSCAPS dscaps;
        dscaps.dwSize = sizeof(dscaps);

        IDirectSound_GetCaps(lpDS, &dscaps);
        AddCapsToLV((CAPDEF *)lParam2, (LPVOID)&dscaps);
    }
}


//================================================================
//================================================================
BOOL DSPrintCaps(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo)
{
    // lParam1 is the GUID for the driver we should open
    // lParam2 is the CAPDEF table we should use

    if (DSCreate((GUID*)lParam1))
    {
        DSCAPS dscaps;
        dscaps.dwSize = sizeof(dscaps);

        IDirectSound_GetCaps(lpDS, &dscaps);

        // Print DirectSound Caps
        return PrintCapsToDC((CAPDEF *)lParam2, (LPVOID)&dscaps, lpInfo);
    }

    // DSCreate failed but keep printing
    return TRUE;
}


//================================================================
//================================================================
void DSCAddCaps(LPARAM lParam1, LPARAM lParam2)
{
    // lParam1 is the GUID for the driver we should open
    // lParam2 is the CAPDEF table we should use

    if (DSCCreate((GUID*)lParam1))
    {
        DSCCAPS dsccaps;
        dsccaps.dwSize = sizeof(dsccaps);

        IDirectSoundCapture_GetCaps(lpDSC, &dsccaps);

        AddCapsToLV((CAPDEF *)lParam2, (LPVOID)&dsccaps);
    }
}


//================================================================
//================================================================
BOOL DSCPrintCaps(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo)
{
    // lParam1 is the GUID for the driver we should open
    // lParam2 is the CAPDEF table we should use

    if (DSCCreate((GUID*)lParam1))
    {
        DSCCAPS dsccaps;
        dsccaps.dwSize = sizeof(dsccaps);

        IDirectSoundCapture_GetCaps(lpDSC, &dsccaps);

        // Print DirectSound Caps
        return PrintCapsToDC((CAPDEF *)lParam2, (LPVOID)&dsccaps, lpInfo);
    }

    // DSCCreate failed but keep printing
    return TRUE;
}


//================================================================
//================================================================
HRESULT CALLBACK DSEnumCallBack(const GUID *lpGUID, LPCTSTR lpDriverDesc, LPCTSTR lpDriverName, LPVOID lpContext)
{
    HTREEITEM hParent = (HTREEITEM)lpContext;
    TCHAR     szText[256];
    LPGUID    lpTemp = NULL;

    if( lpGUID != NULL )
    {
        if(( lpTemp = LocalAlloc( LPTR, sizeof(GUID))) == NULL )
            return( TRUE );

        memcpy( lpTemp, lpGUID, sizeof(GUID));
    }

    // Add subnode to treeview
    if (lpDriverName && *lpDriverName)
        wsprintf(szText, TEXT("%s (%s)"), lpDriverDesc, lpDriverName);
    else
        lstrcpy(szText, lpDriverDesc);

    DSCapDefs[0].szName = szText;
    AddCapsToTV(hParent, DSCapDefs, (LPARAM)lpTemp);

    return(DDENUMRET_OK);
}


//================================================================
//================================================================
HRESULT CALLBACK DSCEnumCallBack(const GUID *lpGUID, LPCTSTR lpDriverDesc, LPCTSTR lpDriverName, LPVOID lpContext)
{
    HTREEITEM hParent = (HTREEITEM)lpContext;
    TCHAR     szText[256];
    LPGUID    lpTemp = NULL;

    if( lpGUID != NULL )
    {
        if(( lpTemp = LocalAlloc( LPTR, sizeof(GUID))) == NULL )
            return( TRUE );

        memcpy( lpTemp, lpGUID, sizeof(GUID));
    }

    // Add subnode to treeview
    if (lpDriverName && *lpDriverName)
        wsprintf(szText, TEXT("%s (%s)"), lpDriverDesc, lpDriverName);
    else
        lstrcpy(szText, lpDriverDesc);

    DSCCapDefs[0].szName = szText;
    AddCapsToTV(hParent, DSCCapDefs, (LPARAM)lpTemp);

    return(DDENUMRET_OK);
}

