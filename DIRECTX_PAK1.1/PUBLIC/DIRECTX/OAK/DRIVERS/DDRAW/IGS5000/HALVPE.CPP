// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  Copyright (c) 1997-2000 Microsoft Corporation
//  COPYRIGHT (C) 1999-2000 IGS TECHNOLOGIES, INC. (iGST)
//
// -----------------------------------------------------------------------------

#include "precomp.h"
#include <nkintr.h>
#include <wavedev.h>
#include <mmddk.h>
#include "IrqMap.h"

WORD wDoubleBuffer = 0;
WORD wInterleave = 0;
WORD wVPEOn = 0;
DWORD dwVBIBufferSize = 0x10000;

HANDLE g_hVideoInterrupt;	// Event from ISR to IST
HANDLE g_hThreadStopped;	// Event to signal that IST exit

BOOL   g_fStopThread;
DWORD g_VideoIRQ;

extern UCHAR Interrupt;

//***************************************************************************
// Audio Driver Interrupt Sharing
//
// The Si* functions below are designed to allow the IGS5000 display driver
// to cooperate with the IGS5000 wavedev driver and share the interrupt.
//
// We rely on the fact that the wavedev driver is alwasy loaded first.
// The audio driver is the implements "primary" interrupt handler, 
// and exposes some proprietary IOCTLS that lets the display driver
// install itself as a "secondary" interrupt handler.
//
// When the video port is enabled, it looks for the appropriate wavedev
// driver. If the IGS5000 wavedev driver is present and running, we
// ask forward any unexpected interrupts to the display driver.
//
//
//***************************************************************************

HANDLE g_hWavedev;
BOOL g_fSharingInterrupt;

#define SIPRIV_OPEN     0x1000
#define SIPRIV_INIT     0x1001
#define SIPRIV_DISABLE  0x1002

#define SIPRIV_ERROR    0xffff
#define SIPRIV_OK       0xfffe


BOOL SiWavedevPrivateMessage (DWORD dwMsg, DWORD dwParm1)
{ BOOL fSuccess;
  DWORD dwReturn;
  DWORD dwTmp;
  DWORD dwStatus;
  MMDRV_MESSAGE_PARAMS msgparams = {0, dwMsg, 0, dwParm1, (DWORD) &dwStatus};

   fSuccess = DeviceIoControl( 
						  g_hWavedev
						, IOCTL_WAV_MESSAGE
						,(LPVOID) &msgparams
						, sizeof(msgparams)
						, &dwReturn
						, sizeof(dwReturn)
						, &dwTmp
						, NULL
						);

   return fSuccess && dwStatus == SIPRIV_OK;
}


void SiInitializeSharedInterrupt (void)
{
    // We'll try to hook up with the wavedev driver, but
    // until then, assume that we're NOT sharing the interrupt
    g_fSharingInterrupt = FALSE;

    // open the wave driver we share an interrupt with
    // Experiment 3: can I use CreateFile/DeviceIoControl to get at the real wave device
	g_hWavedev = CreateFile (TEXT("WAV9:"), GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL, OPEN_EXISTING, 0, 0);

    if (g_hWavedev == INVALID_HANDLE_VALUE) {
        // no such device - we're not sharing interrupts
        return;
    }

    if ( ! SiWavedevPrivateMessage (SIPRIV_OPEN, 0)) {
        // there's a wavedev driver there, but it doesn't know about SIPRIV messages
        return;
    }
    // the interrupt-sharing wavedev driver is present and responding
    g_fSharingInterrupt = TRUE;

}

BOOL SiMapInterruptToSystem(DWORD dwIrq, PDWORD pdwInterruptNumber)
{ BOOL result;

    result = TRUE; // assume all is OK
    if (!g_fSharingInterrupt) {
        // if sharing the interrupt, we don't have to do anything
        result = MapInterruptToSystem(dwIrq, 0, pdwInterruptNumber);
    }
    return result;
}

void SiInterruptDone (DWORD dwInterruptNumber)
{
    if (!g_fSharingInterrupt) {
        // if sharing the interrupt, nothing to do here - we assume the audio driver
        // has already called InterruptDone on our behalf.
        // if not sharing the interrupt, do the real thing
        InterruptDone (dwInterruptNumber);
    }
}

// associate an event handle with an interrupt number
// when the interrupt fires, the event gets signaled.
BOOL SiInterruptInitialize(DWORD dwInterruptNumber, HANDLE hInterruptEvent)
{ BOOL result;

    if (g_fSharingInterrupt) {
        result = SiWavedevPrivateMessage(SIPRIV_INIT, (DWORD) hInterruptEvent);
    }
    else {
        result = InterruptInitialize(dwInterruptNumber, hInterruptEvent, 0, 0);
    }
    return result;
}

void SiInterruptDisable(DWORD dwInterruptNumber)
{
    if (g_fSharingInterrupt) {
        SiWavedevPrivateMessage(SIPRIV_DISABLE, 0);
    }
    else {
        InterruptDisable(dwInterruptNumber);
    }
}

//***************************************************************************
//***************************************************************************

/* data */
const DDPIXELFORMAT InputFormats[] = {
	{sizeof(DDPIXELFORMAT),DDPF_FOURCC, mmioFOURCC('Y','U','Y','V'),16,(DWORD)-1,(DWORD)-1,(DWORD)-1},
    {sizeof(DDPIXELFORMAT),DDPF_FOURCC, mmioFOURCC('U','Y','V','Y'),16,(DWORD)-1,(DWORD)-1,(DWORD)-1}
};
#define MAX_VIDEO_INPUT_FORMATS 	sizeof(InputFormats)/sizeof(DDPIXELFORMAT)


const DDPIXELFORMAT OutputFormats[] = {
    {sizeof(DDPIXELFORMAT),DDPF_FOURCC, mmioFOURCC('Y','U','Y','V'),16,(DWORD)-1,(DWORD)-1,(DWORD)-1},
    {sizeof(DDPIXELFORMAT),DDPF_FOURCC, mmioFOURCC('V','B','I','D'),8,(DWORD)-1,(DWORD)-1,(DWORD)-1}
};
#define MAX_VIDEO_OUTPUT_FORMATS	sizeof(OutputFormats)/sizeof(DDPIXELFORMAT)

const GUID *SupportedGUID[] = {
	&DDVPTYPE_CCIR656,
    &DDVPTYPE_BROOKTREE,
    &DDVPTYPE_PHILIPS,
	&DDVPTYPE_E_HREFH_VREFL,
	&DDVPTYPE_E_HREFH_VREFH,
    &DDVPTYPE_E_HREFL_VREFL,
    &DDVPTYPE_E_HREFL_VREFH
};
#define MAX_SUPPORTED_DDVPTYPE	sizeof(SupportedGUID)/sizeof(void*)

uchar gVBIBuf[MAXVBI_BUFFER_SIZE];

ULONG CALLBACK
VideoIsrThread(PVOID pv) {

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // The thread can be exitted by setting g_fStopThread.
    while (!g_fStopThread) {
	// We spend most of our time waiting for a Video event here.
	WaitForSingleObject (g_hVideoInterrupt, INFINITE);
		memcpy (gVBIBuf, g_pVideoMemory + g_nVideoMemorySize - MAXVBI_BUFFER_SIZE, dwVBIBufferSize);
		///memset(g_pVideoMemory + g_nVideoMemorySize - VBI_BUFFER_SIZE, 0, VBI_BUFFER_SIZE);
//		memset (gVBIBuf, 0, VBI_BUFFER_SIZE);
		//DEBUGMSG(1,(TEXT("+")));
		VBI_ResetIntr();
		SiInterruptDone (g_VideoIRQ);
    }

    // Signal to Destroy that we are exiting now, so it's safe to cleanup.
    SetEvent (g_hThreadStopped);
    return 0;
}

/* helper */
int IsVPTypeSupported(GUID *pguid) {

	for (int i=0; i<MAX_SUPPORTED_DDVPTYPE; i++) {
		if (memcmp(SupportedGUID[i],pguid,sizeof(GUID))==0) return i;
	}
	return -1;
}



/* code */
DWORD WINAPI HalCanCreateVideoPort(LPDDHAL_CANCREATEVPORTDATA pd) {
/*
typedef struct _DDHAL_CANCREATEVPORTDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDVIDEOPORTDESC	    lpDDVideoPortDesc;
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_CANCREATEVIDEOPORT CanCreateVideoPort; // PRIVATE: ptr to callback
} DDHAL_CANCREATEVPORTDATA;
*/

	//Well, We do.
	DEBUGENTER( HalCanCreateVideoPort );
	//DebugBreak();

    /*
     * If the lpInput->lpDDVideoPortDesc can not be supported, set
     * lpInput->ddRVal to the correct error code and return
     * DDHAL_DRIVER_HANDLED
     */
	LPDDVIDEOPORTCONNECT pvpd = &pd->lpDDVideoPortDesc->VideoPortType;
    if (IsVPTypeSupported(&pvpd->guidTypeID) == -1) {
	pd->ddRVal = DDERR_INVALIDDIRECTDRAWGUID;  // TBD: Better error code
	return DDHAL_DRIVER_HANDLED;
    }

	/*
	 * We can check videoport surf size here, and reject if it's too big, or not enough
	 * FIFO, or wrong type, or anything that you can think of.
	 */

	wVPEOn = 0;
	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalCreateVideoPort(LPDDHAL_CREATEVPORTDATA pd) {

/*
typedef struct _DDHAL_CREATEVPORTDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDVIDEOPORTDESC	    lpDDVideoPortDesc;
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port created
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_CREATEVIDEOPORT CreateVideoPort;     // PRIVATE: ptr to callback
} DDHAL_CREATEVPORTDATA;
*/

	//
	DEBUGENTER(HalCreateVideoPort);
	//DebugBreak();

	BOOL	bOk;
	HANDLE	hIsrThread;

    // set up for sharing the IRQ with the Audio driver, if appropriate
    SiInitializeSharedInterrupt ();


	g_hVideoInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_hVideoInterrupt == NULL) {
		ERRORMSG(1, (TEXT("Could not create event handle for video interrupt!!!\r\n")));
		goto error;
	}

	g_hThreadStopped = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (g_hThreadStopped == NULL) {
		ERRORMSG(1, (TEXT("Could not create event handle for video interrupt!!!\r\n")));
		goto error;
	}
	g_fStopThread = FALSE;

	hIsrThread = CreateThread(NULL, 0, VideoIsrThread, 0, 0, NULL);
	if (hIsrThread == NULL) {
		ERRORMSG(1, (TEXT("Failed to create Video ISR thread.\r\n")));
		goto error;
	}
	else {
		// We don't need the thread handle we just got, so we're going to
		// close it right away.
		CloseHandle(hIsrThread);
		hIsrThread = NULL;
	}

	//g_VideoIRQ = MapIrq2SysIntr(9);
        bOk = SiMapInterruptToSystem(Interrupt, &g_VideoIRQ);
        if (!bOk) {
                ERRORMSG(1, (TEXT("MapInterruptToSystem failed!!!\r\n")));
		goto error;
	}
	bOk = SiInterruptInitialize(g_VideoIRQ, g_hVideoInterrupt);
	if (!bOk) {
		ERRORMSG(1, (TEXT("InterruptInitialize(systemVideoIRQ) failed!!!\r\n")));
		goto error;
	}
	///InterruptDone(g_VideoIRQ);
	VBI_InterruptSet (TRUE);

	DWORD dwTemp, dwTemp1;
	WORD wPitch;
	WORD wVBIWidth;

	dwVBIBufferSize = (VBI_ENDLINE - VBI_STARTLINE) * VBI_WIDTH;
	dwTemp = g_nVideoMemorySize - MAXVBI_BUFFER_SIZE;		// Start of VBI region in the frame buffer
	dwTemp1 = g_nVideoMemorySize - 1 ;
	wPitch = wVBIWidth = VBI_WIDTH;

	VPEVBI_SetVBIDst(dwTemp, dwTemp1, wPitch);
	/*We have to allocate two more line buffer and capture two more lines here*/
	/*for philips 7114, line 0xb refers to line 21 of the real video*/
	VPEVBI_SetVBISrc(0, wVBIWidth, VBI_STARTLINE, VBI_ENDLINE);
	VPEVBI_Enable(TRUE);

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
error:
	pd->ddRVal = DDERR_GENERIC;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalFlipVideoPort(LPDDHAL_FLIPVPORTDATA pd) {

/*
typedef struct _DDHAL_FLIPVPORTDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	LPDDRAWI_DDRAWSURFACE_LCL   lpSurfCurr; 	    // current surface
	LPDDRAWI_DDRAWSURFACE_LCL   lpSurfTarg; 	    // target surface
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_FLIP	    FlipVideoPort;  // PRIVATE: ptr to callback
} DDHAL_FLIPVPORTDATA;
*/

	DEBUGENTER(HalFlipVideoPort);
	//DebugBreak();

	long TargOfs	= pd->lpSurfTarg->lpGbl->fpVidMem - (DWORD)g_pVideoMemory;

	WORD wTargPitch = (WORD) pd->lpSurfTarg->lpGbl->lPitch;

	VPE_SetDst(TargOfs,wTargPitch);
	WAIT_FOR_VBLANK;

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortBandwidth(LPDDHAL_GETVPORTBANDWIDTHDATA pd) {
/*
typedef struct _DDHAL_GETVPORTBANDWIDTHDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	LPDDPIXELFORMAT 	    lpddpfFormat;	    // Format for bandwidth
	DWORD			    dwWidth;
	DWORD			    dwHeight;
	DWORD			    dwFlags;		    // Prescale factor for bandwidth
	LPDDVIDEOPORTBANDWIDTH	    lpBandwidth;	    // Returned bandwidth parameters
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETBANDWIDTH GetVideoPortBandwidth;  // PRIVATE: ptr to callback
} DDHAL_GETVPORTBANDWIDTHDATA;
*/
	DEBUGENTER(HalGetVideoPortBandwidth);

	if(pd->dwFlags & DDVPB_TYPE) {
		pd->lpBandwidth->dwCaps = DDVPBCAPS_SOURCE;
	}

    if (pd->dwFlags & DDVPB_VIDEOPORT) {
		DEBUGMSG(1,(TEXT("Bandwidth for videoport\r\n")));
		//implement this if we support DDVPBCAPS_DESTINATION
	}

	if (pd->dwFlags & DDVPB_OVERLAY) {
		DEBUGMSG(1,(TEXT("Bandwidth for overlay\r\n")));
		pd->lpBandwidth->dwOverlay			= 1000;
		pd->lpBandwidth->dwColorkey			= 1000;
		pd->lpBandwidth->dwYInterpolate 	= 1000;
		pd->lpBandwidth->dwYInterpAndColorkey=1000;
	}

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortInputFormats(LPDDHAL_GETVPORTINPUTFORMATDATA pd) {
/*
typedef struct _DDHAL_GETVPORTINPUTFORMATDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	DWORD			    dwFlags;		    // VBI, regular, or both
	LPDDPIXELFORMAT 	    lpddpfFormat;	    // Array of formats
	DWORD			    dwNumFormats;	    // # of formats in array
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETINPUTFORMATS GetVideoPortInputFormats; // PRIVATE: ptr to callback
} DDHAL_GETVPORTINPUTFORMATDATA;
*/
	DEBUGENTER(HalGetVideoPortInputFormats);
	//DebugBreak();

    pd->dwNumFormats = MAX_VIDEO_INPUT_FORMATS;

    if (pd->lpddpfFormat) {
		DEBUGMSG(1,(TEXT("Fill input format to buffer\r\n")));
	memcpy (pd->lpddpfFormat, InputFormats, (sizeof(DDPIXELFORMAT)*pd->dwNumFormats));
    }
    else {
		DEBUGMSG(1,(TEXT("Need only number of input format\r\n")));
    }


	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortOutputFormats(LPDDHAL_GETVPORTOUTPUTFORMATDATA pd) {
/*
typedef struct _DDHAL_GETVPORTOUTPUTFORMATDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	DWORD			    dwFlags;		    // VBI, regular, or both
	LPDDPIXELFORMAT 	    lpddpfInputFormat;	    // Input format
	LPDDPIXELFORMAT 	    lpddpfOutputFormats;    // Array of output formats
	DWORD			    dwNumFormats;	    // # of formats in array
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETOUTPUTFORMATS GetVideoPortOutputFormats; // PRIVATE: ptr to callback
} DDHAL_GETVPORTOUTPUTFORMATDATA;
*/

	DEBUGENTER(HalVideoPortOutputFormats);
	//DebugBreak();

	//Actually, we should validate input format first.

	//Base on input format, we decide which output format is supported.
	pd->dwNumFormats = 0;
	if (pd->dwFlags & DDVPFORMAT_VIDEO) {
		pd->dwNumFormats++;
		if (pd->lpddpfOutputFormats) {
		    memcpy (pd->lpddpfOutputFormats, &OutputFormats[0], sizeof (DDPIXELFORMAT));
		}
	}

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortField(LPDDHAL_GETVPORTFIELDDATA pd) {
/*
typedef struct _DDHAL_GETVPORTFIELDDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	BOOL			    bField;		    // TRUE if even
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETFIELD     GetVideoPortField;	    // PRIVATE: ptr to callback
} DDHAL_GETVPORTFIELDDATA;
*/

	DEBUGENTER(HalGetVideoPortField);
//	DebugBreak();

        int wGUID = IsVPTypeSupported(&(pd->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID));

	if (wGUID == -1) {
		pd->ddRVal = DDERR_VIDEONOTACTIVE;
	}
	else if (VPE_DetectVideoSignal(wGUID)) {
		pd->bField = VPE_CurrField();
	        pd->ddRVal = DD_OK;
	}
	else {
		pd->ddRVal = DDERR_VIDEONOTACTIVE;
	}
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortLine(LPDDHAL_GETVPORTLINEDATA pd) {
/*
typedef struct _DDHAL_GETVPORTLINEDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	DWORD			    dwLine;		    // Current line counter
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETLINE	    GetVideoPortLine;	    // PRIVATE: ptr to callback
} DDHAL_GETVPORTLINEDATA;
*/

	DEBUGENTER(HalGetVideoPortLine);
//	DebugBreak();

    /*
     * (1)Make sure the video port is ON.  If not, set
     * lpInput->ddRVal to DDERR_VIDEONOTACTIVE and return.
     * (2)Readback the current line number and put it in
     * lpInput->dwLine
     */

        int wGUID = IsVPTypeSupported(&(pd->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID));

	if (wGUID == -1) {
		pd->ddRVal = DDERR_VIDEONOTACTIVE;
	}
	else if (VPE_DetectVideoSignal(wGUID)) {
		pd->dwLine = VPE_CurrCapLine();
	        pd->ddRVal = DD_OK;
	}
	else {
		pd->ddRVal = DDERR_VIDEONOTACTIVE;
	}
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortConnectInfo(LPDDHAL_GETVPORTCONNECTDATA pd) {
/*
typedef struct _DDHAL_GETVPORTCONNECTDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	DWORD			    dwPortId;		    // ID of desired video port
	LPDDVIDEOPORTCONNECT	    lpConnect;		    // Array of DDVIDEOPORTCONNECT structures
	DWORD			    dwNumEntries;	    // # of structures in array
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETVPORTCONNECT GetVideoPortConnectInfo; // PRIVATE: ptr to callback
} DDHAL_GETVPORTCONNECTDATA;
*/

	LPDDVIDEOPORTCONNECT	pcn = pd->lpConnect;

	DEBUGENTER(HalGetVideoPortConnectInfo);
	//DebugBreak();

	pd->dwNumEntries = MAX_SUPPORTED_DDVPTYPE;

    /*
     * If lpInput->lpGUIDs is NULL, simply return the number of GUIDS
     * supported
	 * Otherwise, copy the supported GUIDs into the specifed buffer.
	 * The HAL is guarenteed that the buffer is large enough to hold
	 * the GUIDs.
     */
    if (pcn) {
		for (int i=0; i< MAX_SUPPORTED_DDVPTYPE; i++) {
			pcn->dwSize			= sizeof(DDVIDEOPORTCONNECT);
			pcn->dwPortWidth	= 8;
			memcpy(&pcn->guidTypeID, SupportedGUID[i], sizeof(GUID));
		pcn->dwFlags		= DDVPCONNECT_INTERLACED		|
								  //DDVPCONNECT_INVERTPOLARITY	|
								  DDVPCONNECT_VACT				|
								  DDVPCONNECT_DISCARDSVREFDATA	|
								  0;

			pcn++;
		}
    }


	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalDestroyVideoPort(LPDDHAL_DESTROYVPORTDATA pd) {
/*
typedef struct _DDHAL_DESTROYVPORTDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_DESTROYVPORT DestroyVideoPort;	    // PRIVATE: ptr to callback
} DDHAL_DESTROYVPORTDATA;

*/

	DEBUGENTER(HalDestroyVideoPort);

	SiInterruptDisable(g_VideoIRQ);
	VBI_InterruptSet (FALSE);

	g_fStopThread = TRUE;
    WaitForSingleObject(g_hThreadStopped, 2000);  // Don't wait forever.

    // Close all open event handler
	CloseHandle(g_hThreadStopped);
	CloseHandle(g_hVideoInterrupt);

	VPEVBI_Enable(FALSE);
	VPE_EnableOverDoubleBuffer(0);
	VPE_EnableVPDoubleBuffer(0);

	wVPEOn = 0;
	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalGetVideoPortFlipStatus(LPDDHAL_GETVPORTFLIPSTATUSDATA pd) {
/*
typedef struct _DDHAL_GETVPORTFLIPSTATUSDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	FLATPTR 		    fpSurface;		    // surface struct
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETFLIPSTATUS GetVideoPortFlipStatus; // PRIVATE: ptr to callback
} DDHAL_GETVPORTFLIPSTATUSDATA;
*/

	DEBUGENTER(HalGetVideoPortFlipStatus);
	//DebugBreak();

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalUpdateVideoPort(LPDDHAL_UPDATEVPORTDATA pd) {
/*
typedef struct _DDHAL_UPDATEVPORTDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	LPDDRAWI_DDRAWSURFACE_INT   *lplpDDSurface;	    // surface struct
	LPDDRAWI_DDRAWSURFACE_INT   *lplpDDVBISurface;	    // VBI surface structure
	LPDDVIDEOPORTINFO	    lpVideoInfo;	    // Video information
	DWORD			    dwFlags;		    // DDRAWI_VPORTSTART, DDRAWI_VPORTSTOP, DDRAWI_VPORTUPDATE
	DWORD			    dwNumAutoflip;	    // # of autoflip surfaces. If > 1, lpDDSurface and lpDDVBISurface are arrays.
	DWORD			    dwNumVBIAutoflip;	    // # of autoflip surfaces. If > 1, lpDDSurface and lpDDVBISurface are arrays.
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_UPDATE	    UpdateVideoPort;	    // PRIVATE: ptr to callback
} DDHAL_UPDATEVPORTDATA;
*/


	DEBUGENTER(HalUpdateVideoPort);
	//DebugBreak();

	if (pd->dwFlags == DDRAWI_VPORTSTOP) {
		VPE_Off();
		wVPEOn = 0;
		pd->ddRVal = DD_OK;
		return DDHAL_DRIVER_HANDLED;
	}
	if (pd->dwFlags == DDRAWI_VPORTSTART) {
		if(wVPEOn == 0) {
			VPE_Init();
			VPE_ConnectDecoder(IsVPTypeSupported(&pd->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID));
		}
	}

	//when they STOP VIDEOPORT, lplpDDSurface is NULL :-)
	LPDDRAWI_DDRAWSURFACE_INT	pDstInt = *pd->lplpDDSurface;
	LPDDRAWI_DDRAWSURFACE_LCL	pDstSurfLcl = pDstInt->lpLcl;
	LPDDRAWI_DDRAWSURFACE_GBL	pDstSurfGbl = pDstSurfLcl->lpGbl;

	WORD wDstBytePP = (WORD)(pd->lpVideoInfo->lpddpfInputFormat->dwYUVBitCount / 8);
    WORD wDstStride = pDstSurfGbl->wWidth * wDstBytePP;
	WORD wDstPitch	= (WORD)pDstSurfGbl->lPitch;
	long DstOfs		= pDstSurfGbl->fpVidMem - (DWORD)g_pVideoMemory;
	WORD wCaptureTwoFields= (WORD)(pd->lpVideoPort->ddvpDesc.VideoPortType.dwFlags & DDVPCONNECT_INTERLACED);
	WORD wCropL, wCropT, wCropR, wCropB;

	WORD wDstW, wDstH;
	WORD wSrcW, wSrcH;

	if(wCaptureTwoFields && (pd->lpVideoInfo->dwVPFlags & DDVP_SKIPEVENFIELDS ||
		pd->lpVideoInfo->dwVPFlags & DDVP_SKIPODDFIELDS))
		wCaptureTwoFields = 0;/*Skip field*/

	wDoubleBuffer = 0;

    if (pd->lpVideoInfo->dwVPFlags & DDVP_AUTOFLIP)
    {
		wDoubleBuffer = 1;

	if (pd->dwNumAutoflip == 2)
			wDoubleBuffer = 1;
		else
			wDoubleBuffer = 0;
    }
	else
		wDoubleBuffer = 0;

	if(wDoubleBuffer)/*if using double buffer, we can't support skipping field*/
		wCaptureTwoFields = 1;/*so, force it to full fields.*/

	if (pd->lpVideoInfo->dwVPFlags & DDVP_INTERLEAVE) {
		wInterleave = 1;
		wCaptureTwoFields = 1;
		wDoubleBuffer = 0;/*we can't do double buffer and interleave simultaneously*/
	}
	else
		wInterleave = 0;

	// adjust src ptr
    DstOfs += pd->lpVideoInfo->dwOriginX * wDstBytePP;
	DstOfs += pd->lpVideoInfo->dwOriginY * wDstPitch;

	if (pd->dwFlags == DDRAWI_VPORTUPDATE || pd->dwFlags == DDRAWI_VPORTSTART) {

		//about crop.
		wSrcW	= (WORD) pd->lpVideoPort->ddvpDesc.dwFieldWidth;
		wSrcH	= (WORD) pd->lpVideoPort->ddvpDesc.dwFieldHeight;

		WORD leftCrop720 = 0;
		WORD topCrop260 = 0;
		if (wSrcW < 720)
			leftCrop720 = (WORD)((720 - pd->lpVideoPort->ddvpDesc.dwFieldWidth) / 2 * 2);
		if (wSrcH < 260)
			topCrop260 = 16;

		if (pd->lpVideoInfo->dwVPFlags & DDVP_CROP) {
			wCropL = (WORD) pd->lpVideoInfo->rCrop.left*2 + leftCrop720;
			wCropR = (WORD) pd->lpVideoInfo->rCrop.right*2 + leftCrop720;
			wCropT = (WORD) pd->lpVideoInfo->rCrop.top + topCrop260;
			wCropB = (WORD) pd->lpVideoInfo->rCrop.bottom + topCrop260;
			wSrcW = (WORD)(pd->lpVideoInfo->rCrop.right - pd->lpVideoInfo->rCrop.left);
			wSrcH = (WORD)(pd->lpVideoInfo->rCrop.bottom - pd->lpVideoInfo->rCrop.top);
	}
		else {
			wCropL = 0 + leftCrop720;
			wCropT = 0 + topCrop260;
			wCropR = (WORD)(pd->lpVideoPort->ddvpDesc.dwFieldWidth*2) + leftCrop720;
			wCropB = (WORD)(pd->lpVideoPort->ddvpDesc.dwFieldHeight/* + 0xc*/) + topCrop260;
		}
		VPE_SetCrop(wCropL, wCropT, wCropR, wCropB);

		//about prescale
		if (pd->lpVideoInfo->dwVPFlags & DDVP_PRESCALE) {
			wDstW	= (WORD) pd->lpVideoInfo->dwPrescaleWidth;
			wDstH	= (WORD) pd->lpVideoInfo->dwPrescaleHeight;
		}
		else {
			wDstW	= wSrcW;
			wDstH	= wSrcH;
		}

		VPE_DDA(wSrcW, wDstW, wSrcH, wDstH);
		VPE_DDA_On();
	}
	//non/interlaced source?
	if (wCaptureTwoFields) {
		VPE_InterlacedSrc();
	}
	else {
		VPE_NonInterlacedSrc();
	}

	//if connection is INVERTPOLARITY, we do support it
	if (pd->lpVideoPort->ddvpDesc.VideoPortType.dwFlags & DDVPCONNECT_INVERTPOLARITY) {
		VPE_InvertPolarity_On();
	}
	else {
		VPE_InvertPolarity_Off();
	}

	//want mirror?
	if (pd->lpVideoInfo->dwVPFlags & DDVP_MIRRORLEFTRIGHT) {
		VPE_XMirror_On();
		DstOfs += (wDstPitch - wDstBytePP);
	}
	else {
		VPE_XMirror_Off();
	};
	if (pd->lpVideoInfo->dwVPFlags & DDVP_MIRRORUPDOWN) {
		//not only set this but also adjust dst ptr
		VPE_YMirror_On();
		if(wInterleave != 0) /*03/28/2000*/
			DstOfs += (wDstH * 2 - 1)* wDstPitch;
		else
			DstOfs += (wDstH - 1)* wDstPitch;
	}
	else {
		VPE_YMirror_Off();
	}

    if (wDoubleBuffer) /*if we are using double buffer, set destination for the back buffer*/
	{
	    /*
	    ** If hardware double buffering, set base address
	    ** for second buffer
	    */
			DWORD dwTemp;
			VPE_SetDst(DstOfs, wDstPitch);
	    dwTemp = pd->lplpDDSurface[1]->lpLcl->lpGbl->fpVidMem - (DWORD)g_pVideoMemory;
		    dwTemp += pd->lpVideoInfo->dwOriginX * wDstBytePP;
			dwTemp += pd->lpVideoInfo->dwOriginY * wDstPitch;
			if (pd->lpVideoInfo->dwVPFlags & DDVP_MIRRORLEFTRIGHT) {
				dwTemp += (wDstPitch - wDstBytePP);
			}
			if (pd->lpVideoInfo->dwVPFlags & DDVP_MIRRORUPDOWN) {
			//not only set this but also adjust dst ptr
				dwTemp += (wDstH - 1)* wDstPitch;
			}

			VPE_SetVPBackBufferDst(dwTemp);
			VPE_EnableVPDoubleBuffer(1);
	}
	else if(wCaptureTwoFields && wInterleave == 0) {/*Capture odd/even fields to same location*/
		wDoubleBuffer = 1;
		VPE_SetDst(DstOfs, wDstPitch);
		VPE_SetVPBackBufferDst(DstOfs);/*Enable capture double buffer feature, while set back buffer = Dst buffer*/
		VPE_EnableVPDoubleBuffer(1);
	}
	else {
		VPE_SetDst(DstOfs, wDstPitch);
		VPE_EnableVPDoubleBuffer(0);
	}

	//discard vref or not?
	if (pd->lpVideoPort->ddvpDesc.VideoPortType.dwFlags & DDVPCONNECT_DISCARDSVREFDATA) {
		VPE_DiscardVRef();
	};
	if (pd->dwFlags == DDRAWI_VPORTSTART) {
		if(wVPEOn == 0) {
			//VPE_Init();
			//VPE_ConnectDecoder(IsVPTypeSupported(&pd->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID));
			VPE_On();
			wVPEOn = 1;
		}
	}
	//finish now?
	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalWaitForVideoPortSync(LPDDHAL_WAITFORVPORTSYNCDATA pd) {
/*
typedef struct _DDHAL_WAITFORVPORTSYNCDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	DWORD			    dwFlags;		    // DDVPEVENT_XXXX
	DWORD			    dwLine;
	DWORD			    dwTimeOut;		    // Max time to wait before returning
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_WAITFORSYNC  WaitForVideoPortSync;   // PRIVATE: ptr to callback
} DDHAL_WAITFORVPORTSYNCDATA;
*/
	DEBUGENTER(HalWaitForVideoPortSync);
//	DebugBreak();

	return DDHAL_DRIVER_NOTHANDLED;
};

DWORD WINAPI HalGetVideoSignalStatus(LPDDHAL_GETVPORTSIGNALDATA pd) {
/*
typedef struct _DDHAL_GETVPORTSIGNALDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	DWORD			    dwStatus;		    // Video signal status
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_GETSIGNALSTATUS GetVideoSignalStatus;// PRIVATE: ptr to callback
} DDHAL_GETVPORTSIGNALDATA;
*/

	DEBUGENTER(HalGetVideoSignalStatus);
//	DebugBreak();

	int wGUID = IsVPTypeSupported(&(pd->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID));

        pd->ddRVal = DD_OK;
	if (wGUID == -1) {
		pd->dwStatus = DDVPSQ_NOSIGNAL;
	}
	else if (VPE_DetectVideoSignal(wGUID)) {
		pd->dwStatus = DDVPSQ_SIGNALOK;
	}
	else {
		pd->dwStatus = DDVPSQ_NOSIGNAL;
	}
	return DDHAL_DRIVER_HANDLED;
};

DWORD WINAPI HalColorControl(LPDDHAL_VPORTCOLORDATA pd) {
/*
typedef struct _DDHAL_VPORTCOLORDATA {
	LPDDRAWI_DIRECTDRAW_LCL     lpDD;		    // driver struct
	LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;	    // Video port object
	DWORD			    dwFlags;		    // Video signal status
	LPDDCOLORCONTROL	    lpColorData;
	HRESULT 		    ddRVal;		    // return value
	LPDDHALVPORTCB_COLORCONTROL ColorControl;	    // PRIVATE: ptr to callback
} DDHAL_VPORTCOLORDATA;
*/
	DEBUGENTER(HalColorControl);
//	DebugBreak();

	return DDHAL_DRIVER_NOTHANDLED;
};
