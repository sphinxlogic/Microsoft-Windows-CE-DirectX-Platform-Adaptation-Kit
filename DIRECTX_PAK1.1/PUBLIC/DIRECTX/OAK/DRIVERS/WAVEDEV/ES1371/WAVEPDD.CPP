// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000  Microsoft Corporation
//  
//  Module Name:  
//  
//      wavepdd.c
//  
//  Abstract:  
//      The PDD (Platform Dependent Driver) is responsible for 
//      communicating with the audio circuit to start and stop playback
//      and/or recording and initialize and deinitialize the circuits.
//  
//  Functions:
//      PDD_AudioGetInterruptType
//      PDD_AudioMessage
//      PDD_AudioInitialize
//      PDD_AudioDeinitialize
//      PDD_AudioPowerHandler
//      PDD_WaveProc
//  
//  Notes:
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <nkintr.h>
#include <ceddk.h>
#include "es1371.h"

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("ES1371")
	, {
		 TEXT("Test")
		,TEXT("Params")
		,TEXT("Verbose")
		,TEXT("Interrupt")
		,TEXT("WODM")
		,TEXT("WIDM")
		,TEXT("PDD")
		,TEXT("MDD")
		,TEXT("8")
		,TEXT("Misc")
		,TEXT("10")
		,TEXT("IOcontrol")
		,TEXT("Alloc")
		,TEXT("Function")
		,TEXT("Warning")
		,TEXT("Error")
	}
	, 0xC000
}; 
#endif

// Use define to retirieve Irq and IO Base from registry instead of card
//#define ENSONIQ_USE_REGSISTRY 

// Use memory locations located in config.bib, for contigeuos memory larger than 4k
//#define ENSONIQ_USE_PREALLOCATEDMEMORY

//
// Defined and allocated in Config.bib
//
#ifdef ENSONIQ_USE_PREALLOCATEDMEMORY
#ifdef CEPC
#define AUDIO_DMA_BUFFER_BASE_PA    0x001FC000
#endif

#ifdef PUZZLE
#define AUDIO_DMA_BUFFER_BASE_PA    0X01FD0000
#endif

#ifdef ALTOONA
#define AUDIO_DMA_BUFFER_BASE_PA    0X03BB0000
#endif

//WARNING:  This define is place here only to prevent build breaks
//          If one is using a preallocated buffer a hardcode defined
//          physical memory address must be provided in config.bib
#ifndef AUDIO_DMA_BUFFER_BASE_PA
#define AUDIO_DMA_BUFFER_BASE_PA    0x00000000
#endif
#endif

#define AUDIO_DMA_NUMBER_PAGES		2

#ifdef __cplusplus
extern "C" {
#endif

// Globals 

USHORT   v_nVolume;
ULONG    v_nSetVolume;
BOOL	 fInUse[2];

VOID     AudioPowerOn();

CES1371* pCES1371;
static PBYTE  dma_out_page[AUDIO_DMA_NUMBER_PAGES]; 
static PBYTE  dma_in_page[AUDIO_DMA_NUMBER_PAGES]; 
static ULONG  dma_page_size;
static ULONG  dma_buffer_size;

static PWAVEFORMATEX g_pwfx[2];
static PCM_TYPE g_pcmtype[2];

//
// These variables may be accessed by different processes or the hardware and
// are likely to change in a time critical fashion. Optimization may cause
// errors if not declared volatile.
//
volatile        BOOL    v_fMoreData[2];
volatile static ULONG   v_nNextPage[2];

// Volume Linear to Log Lookup table
// aproximates to roughly 2 db's per increment
UCHAR volumeLUT[] = {
0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x2,
0x2,0x3,0x3,0x3,0x4,0x4,0x5,0x5,
0x6,0x6,0x7,0x8,0x9,0x9,0xA,0xB,
0xC,0xE,0xF,0x11,0x13,0x15,0x19,0x19};


#ifdef ENSONIQ_USE_REGSISTRY 
// -----------------------------------------------------------------------------
//
// Function to get the IRQ and I/O port range from the registry
//
// NOTE: lpRegPath is assumed to be under HKEY_LOCAL_MACHINE
//
// Returns ERROR_SUCCESS on success or a Win32 error code on failure
//
// -----------------------------------------------------------------------------
DWORD
GetRegistryConfig(
    LPWSTR lpRegPath,
    UCHAR * lpdwIrq,
    USHORT * lpdwIoBase
    )
{
    HKEY hConfig = INVALID_HANDLE_VALUE;
    HKEY  hKeyActive = INVALID_HANDLE_VALUE;
    WCHAR szConfigReg[128];
    DWORD dwData = 0;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwRet;

    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //
    //  The HLM\Drivers\Active\?? key was passed in from DEVICE.EXE. The "Key"
    //  subkey points to the original registry location where we can find our
    //  extra data about this device.
    //
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegPath, 0, 0, &hKeyActive);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_PDD, 
             (TEXT("WAVEDEV : GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
            lpRegPath, dwRet));
        goto EXIT;
    }

    dwSize = sizeof(szConfigReg);
    dwRet = RegQueryValueEx(hKeyActive, TEXT("Key"), 0, &dwType, (PUCHAR)szConfigReg, &dwSize);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_PDD,
            (TEXT("WAVEDEV : GetRegistryConfig RegQueryValueEx(Key) failed %d\r\n"), dwRet));
        goto EXIT;
    }


    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szConfigReg, 0, 0, &hConfig);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_PDD, 
             (TEXT("WAVEDEV : GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
            lpRegPath, dwRet));
        goto EXIT;
    }
    
    dwSize = PC_REG_IRQ_VAL_LEN;
    dwRet = RegQueryValueEx(hConfig, PC_REG_IRQ_VAL_NAME, 0, &dwType, 
                            (PUCHAR)&dwData, &dwSize);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_PDD,
            (TEXT("WAVEDEV : GetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
            PC_REG_IRQ_VAL_NAME, dwRet));
        goto EXIT;
    }
    *lpdwIrq = (UCHAR) dwData;
    dwData = 0;

    dwSize = PC_REG_IOBASE_VAL_LEN;
    dwRet = RegQueryValueEx(hConfig, PC_REG_IOBASE_VAL_NAME, 0, &dwType,
                            (PUCHAR)&dwData, &dwSize);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_PDD,
            (TEXT("WAVEDEV : GetRegistryConfig RegQueryValueEx(%s) failed %d\r\n"),
            PC_REG_IOBASE_VAL_NAME, dwRet));
        goto EXIT;
    }
    *lpdwIoBase = (USHORT) dwData;

EXIT:
    if (hConfig != INVALID_HANDLE_VALUE) {
        RegCloseKey(hConfig);
    }
    if (hKeyActive != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyActive);
    }
    return dwRet;
}
#endif



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT
private_WaveGetDevCaps(
    WAPI_INOUT apidir,
    PVOID pCaps,
    UINT  wSize
    )
{
    PWAVEINCAPS pwic  = (PWAVEINCAPS)pCaps;
    PWAVEOUTCAPS pwoc = (PWAVEOUTCAPS)pCaps;

    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WPDD("+PDD_WaveGetDevCaps");

    if (pCaps == NULL)  {
        //
        // If pCaps == NULL, we are requesting if the driver PDD is capable of
        // this mode at all. In other words, if APIDIR == WAPI_IN,  we return
        // no error if input is supported, and MMSYSERR_NOTSUPPORTED otherwise.
        // Since Odo has both, we return MMSYSERR_NOERROR regardless.
        //
        return(MMSYSERR_NOERROR);
    }

    //
    // Fill in the DevCaps here.
    //    
    pwoc->wMid = MM_MICROSOFT;
    pwoc->wPid = (apidir == WAPI_OUT ? 24 : 23);  // generic in or out...
    pwoc->vDriverVersion = 0x0001;

    wsprintf (pwoc->szPname, TEXT("CE/PC Audio (%hs)"), __DATE__);

    pwoc->dwFormats =   WAVE_FORMAT_1M08 | WAVE_FORMAT_1M16 |
                        WAVE_FORMAT_1S08 | WAVE_FORMAT_1S16 |
                        WAVE_FORMAT_2M08 | WAVE_FORMAT_2M16 |
                        WAVE_FORMAT_2S08 | WAVE_FORMAT_2S16 |
                        WAVE_FORMAT_4M08 | WAVE_FORMAT_4M16 |
                        WAVE_FORMAT_4S08 | WAVE_FORMAT_4S16;						

    pwoc->wChannels = 2;
    if (apidir == WAPI_OUT) 
    {
        // Caution: the WAVEINCAPS structure doesn't have a dwSupport field. Touch here and die!
        pwoc->dwSupport = WAVECAPS_VOLUME;
    }

    FUNC_WPDD("-PDD_WaveGetDevCaps");

    return(mmRet);
}    


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
private_AudioFillBuffer (
    PWAVEHDR pwh,
    BYTE UNALIGNED *pBuffer
    )
{
	ULONG offset;
	ULONG bytescopied;
    PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[WAPI_OUT]; 

    FUNC_VERBOSE("+PDD_AudioFillBuffer");

    //
	// Fill the DMA Buffer and clear the remainder if nessary 
    //
	bytescopied = 0;
	offset = 0; 
	while ( offset < dma_page_size ) 
	{
		if (pwh != NULL && pwh->dwBytesRecorded >= pwh->dwBufferLength) 
		{
			pwh = pwh->lpNext;
			continue;
		}

	    if (pwh != NULL) 
		{
			// Fill the DMA Buffer 
			bytescopied = pwh->dwBufferLength - pwh->dwBytesRecorded;
			if ( bytescopied > dma_page_size - offset )
				bytescopied = dma_page_size - offset;

			memcpy(pBuffer + offset, pwh->lpData + pwh->dwBytesRecorded, bytescopied);

			pwh->dwBytesRecorded += bytescopied;
        }
		else
		{
			// Nothing lefted to copy clear the remainder of the buffer
			if ( pFormat->wBitsPerSample == 8 )
			    memset(pBuffer + offset, 0x80, dma_page_size-offset);
			else
			    memset(pBuffer + offset, 0, dma_page_size-offset);

			bytescopied += dma_page_size-offset;
		}

	    offset += bytescopied;
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
private_AudioGetBuffer (
    PWAVEHDR pwh,
    BYTE UNALIGNED *pBuffer
    )
{
	ULONG offset;
	ULONG bytescopied;
    PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[WAPI_IN]; 

    FUNC_VERBOSE("+PDD_AudioGetBuffer");

    //
	// Fill the DMA Buffer and clear the remainder if nessary 
    //
	bytescopied = 0;
	offset = 0; 
	while ( offset < dma_page_size ) 
	{
		if (pwh != NULL && pwh->dwBytesRecorded >= pwh->dwBufferLength) 
		{
			pwh = pwh->lpNext;
			continue;
		}

	    if (pwh == NULL) 
			break;

		// Fill the DMA Buffer 
		bytescopied = pwh->dwBufferLength - pwh->dwBytesRecorded;
		if ( bytescopied > dma_page_size - offset )
			bytescopied = dma_page_size - offset;

		memcpy(pwh->lpData + pwh->dwBytesRecorded, pBuffer + offset, bytescopied);

		pwh->dwBytesRecorded += bytescopied;

	    offset += bytescopied;
	}
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID 
private_WaveOutEndOfData(
	WAPI_INOUT apidir
	)
{
    FUNC_WPDD("+PDD_WaveOutEndOfData");

	PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[apidir]; 

	//
	// There is no more data coming...
	//
	v_fMoreData[apidir] = FALSE;

	if ( apidir == WAPI_OUT )
	{
		//
		// Clear DMA buffer 
		//
		if ( pFormat->wBitsPerSample == 8 )
			memset (dma_out_page[v_nNextPage[WAPI_OUT]], 0x80, dma_page_size);
		else
			memset (dma_out_page[v_nNextPage[WAPI_OUT]], 0, dma_page_size);

		//
		// Stop the DMA channel
		// 
		pCES1371->StopDMAChannel( ES1371_DAC0 );
	}
	else
	{
		//
		// Stop the DMA channel
		// 
		pCES1371->StopDMAChannel( ES1371_ADC );
	}

    FUNC_WPDD("-PDD_WaveOutEndOfData");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
private_WaveOutStart (
    WAPI_INOUT apidir,
    PWAVEHDR pwh
    )
{
    PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[apidir]; 

    FUNC_WPDD("+PDD_WaveOutStart");

    v_fMoreData[apidir] = TRUE;        
    v_nNextPage[apidir] = 0;

	//
	// Fill dma buffers 
	//
	if ( apidir == WAPI_OUT )
	{
		private_AudioFillBuffer(pwh, dma_out_page[0]);
		private_AudioFillBuffer(pwh, dma_out_page[1]);
	}

	//
	// Set Volume 
	// 
	pCES1371->WriteCodecRegister( AC97_MASTER_VOL_STEREO, v_nVolume );

	if ( apidir == WAPI_OUT )
	{
		//
		// Initialize the DMA channel format 
		//
		pCES1371->SetDMAChannelFormat( ES1371_DAC0,
									   pFormat->wf.nChannels,
									   (pFormat->wBitsPerSample==8)?0:1,
									   pFormat->wf.nSamplesPerSec );
		//
		// Initialize the DMA position to fire interrupt
		//
		ULONG num_samples_per_dma_page = dma_page_size/(((pFormat->wBitsPerSample==8)?1:2)*pFormat->wf.nChannels);
		pCES1371->SetDMAChannelBuffer( ES1371_DAC0,
									   dma_buffer_size,
									   num_samples_per_dma_page );

		DEBUGMSG(ZONE_VERBOSE,(TEXT("ENSONIQ1371: Playing sample @ chn[%d] bit[%d] sps[%d] nspp[%d]\n\r"),pFormat->wf.nChannels,pFormat->wBitsPerSample,pFormat->wf.nSamplesPerSec,num_samples_per_dma_page));

		//
		// Start the DMA channel
		// 
		pCES1371->StartDMAChannel( ES1371_DAC0 );
	}
	else
	{
		//
		// Initialize the DMA channel format 
		//
		pCES1371->SetDMAChannelFormat( ES1371_ADC,
									   pFormat->wf.nChannels,
									   (pFormat->wBitsPerSample==8)?0:1,
									   pFormat->wf.nSamplesPerSec );
		//
		// Initialize the DMA position to fire interrupt
		//
		ULONG num_samples_per_dma_page = dma_page_size/(((pFormat->wBitsPerSample==8)?1:2)*pFormat->wf.nChannels);
		pCES1371->SetDMAChannelBuffer( ES1371_ADC,
									   dma_buffer_size,
									   num_samples_per_dma_page );

		DEBUGMSG(ZONE_VERBOSE,(TEXT("ENSONIQ1371: Recording sample @ chn[%d] bit[%d] sps[%d] nspp[%d]\n\r"),pFormat->wf.nChannels,pFormat->wBitsPerSample,pFormat->wf.nSamplesPerSec,num_samples_per_dma_page));

		//
		// Start the DMA channel
		// 
		pCES1371->StartDMAChannel( ES1371_ADC );
	}

    FUNC_WPDD("-PDD_WaveOutStart");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
private_WaveOutContinue (
    WAPI_INOUT apidir,
    PWAVEHDR pwh
    )
{
    FUNC_VERBOSE("+PDD_WaveOutContinue");

	if ( apidir == WAPI_OUT )
	{
		ULONG pos = pCES1371->GetDMAPosition(ES1371_DAC0);
		if ( pos/dma_page_size == v_nNextPage[WAPI_OUT] )
			v_nNextPage[WAPI_OUT] = (!v_nNextPage[WAPI_OUT] ? 1 : 0);

		private_AudioFillBuffer(pwh, dma_out_page[v_nNextPage[WAPI_OUT]]);
	}
	else
	{
		private_AudioGetBuffer(pwh, dma_in_page[v_nNextPage[WAPI_IN]]);
	}

    v_nNextPage[apidir] = (!v_nNextPage[apidir] ? 1 : 0);

    FUNC_VERBOSE("-PDD_WaveOutContinue");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT
private_WaveOutRestart (
    WAPI_INOUT apidir,
    PWAVEHDR pwh
    )
{
    FUNC_VERBOSE("+PDD_WaveOutRestart");

    private_WaveOutStart (apidir, pwh);

    FUNC_VERBOSE("-PDD_WaveOutRestart");

    return(MMSYSERR_NOERROR);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT
private_WaveOutPause (
    WAPI_INOUT apidir
    )
{
    FUNC_VERBOSE("+PDD_WaveOutPause");

    private_WaveOutEndOfData(apidir);

    FUNC_VERBOSE("-PDD_WaveOutPause");

    return(MMSYSERR_NOERROR);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID 
private_WaveOutStop(
    WAPI_INOUT apidir
    )
{
    FUNC_WPDD("+private_WaveOutStop");

	//
	// Stop the DMA channel
	// 
	if ( apidir == WAPI_OUT )
	{
		pCES1371->StopDMAChannel( ES1371_DAC0 );
	}
	else
	{
		pCES1371->StopDMAChannel( ES1371_ADC );
	}
    
    FUNC_WPDD("-private_WaveOutStop");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID 
private_WaveStandby(
    WAPI_INOUT apidir
    )
{
    //
    // We are going to be idle. so power off what we can.
    //
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT
private_WaveOpen(
    WAPI_INOUT apidir,
    LPWAVEFORMATEX lpFormat,
    BOOL fQueryFormatOnly
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WPDD("+PDD_WaveOpen");

    //
    // Allow PCM, mono or stereo, 8 or 16 bit at 11k or 22k Hz
    //
    if ((lpFormat->wFormatTag != WAVE_FORMAT_PCM) ||
        (lpFormat->nChannels  != 1 && lpFormat->nChannels != 2) ||
	    (lpFormat->nSamplesPerSec < 4000 || lpFormat->nSamplesPerSec > 48000) ||
        (lpFormat->wBitsPerSample != 16 && lpFormat->wBitsPerSample != 8) )
    {
        mmRet = WAVERR_BADFORMAT;
        goto EXIT;
    }

	//
	// Only checking if format is supported
	//
    if (fQueryFormatOnly)
	{
        goto EXIT;
    }

    //
    // The hardware can support input and output at the sametime
	// but not multiple output and inputs return MMSYSERR_ALLOCATED 
	// if the input or output is alread in use.
    //
    if (fInUse[apidir]) 
	{
        mmRet = MMSYSERR_ALLOCATED;
        goto EXIT;
    }

    fInUse[apidir] = TRUE;

    // 
	// Save format the device was opened in  
	//
    g_pwfx[apidir] = lpFormat;

EXIT:
    FUNC_WPDD("-PDD_WaveOpen");
    return(mmRet);
}    


void private_waveOutSetVolume(ULONG vol)
{
	USHORT rvol,lvol;
	UCHAR  rindex,lindex;

	lindex = (UCHAR)((0xF8000000 - (vol & 0xF8000000)) >> 27);
	rindex = (UCHAR)((0x0000F800 - (vol & 0x0000F800)) >> 11);

	lvol = volumeLUT[lindex] << 8;
	rvol = volumeLUT[rindex];

	v_nVolume = lvol | rvol; 
	v_nSetVolume = vol;

	// write master volume
	pCES1371->WriteCodecRegister( AC97_MASTER_VOL_STEREO, v_nVolume );
}


// ---------------------------------------------------------------------------
//
// Routine for powering on Audio hardware.  Is called from within power
// handler, so should not make any system calls.
//
// ---------------------------------------------------------------------------
VOID
AudioPowerOn()
{
   v_nNextPage[WAPI_OUT] = 0;
   v_nNextPage[WAPI_IN]  = 2;
   v_fMoreData[WAPI_OUT] = FALSE;
   v_fMoreData[WAPI_IN]  = FALSE;
}


// -----------------------------------------------------------------------------
//							PDD_AudioPowerHandler
// -----------------------------------------------------------------------------
VOID
PDD_AudioPowerHandler(
    BOOL power_down
   )
{
    //
    // Handle any power up/down issues here. 
    //
}


// -----------------------------------------------------------------------------
//                       PDD_AudioGetInterruptType
// -----------------------------------------------------------------------------
AUDIO_STATE 
PDD_AudioGetInterruptType(
   VOID
   )
{

    AUDIO_STATE result = AUDIO_STATE_IGNORE;

    UCHAR intsrc = pCES1371->GetInterruptSource();

    //
    // Which interrupt is the source?
    //
    if ( intsrc & ES1371_INT_DAC0 )
    { 
        //
        // ACK the ISR.
        //
        pCES1371->AckDMAInterrupt( ES1371_INT_DAC0 );

		//
		// Check if there is anything more to play, if not inform the IST
		//
        if (v_fMoreData[WAPI_OUT]) {
			result |= AUDIO_STATE_OUT_PLAYING;
        }
        else {
			result |= AUDIO_STATE_OUT_STOPPED;
        }
    }

    if ( intsrc & ES1371_INT_ADC )
    { 
        //
        // ACK the ISR.
        //
        pCES1371->AckDMAInterrupt( ES1371_INT_ADC );

		//
		// Check if there is anything more to play, if not inform the IST
		//
        if (v_fMoreData[WAPI_IN]) {
			result |= AUDIO_STATE_IN_RECORDING;
        }
        else {
			result |= AUDIO_STATE_OUT_STOPPED;
        }
    }

    if (intsrc & ES1371_INT_PM)
    {
        //
        // ACK the ISR.
        //
        pCES1371->AckDMAInterrupt( intsrc );
        DEBUGMSG(ZONE_ERROR, (TEXT("ES1371: power interrupt ocurred ")));
    }

    //
    // Don't indicate any state change on this interrupt...
    //
    return result;
}


// -----------------------------------------------------------------------------
//								PDD_AudioInitialize
// -----------------------------------------------------------------------------
BOOL 
PDD_AudioInitialize(
    DWORD dwIndex   
   )
{
	ULONG i;
    BOOL  bRetVal = 0;
    ULONG nRetVal = 0;
#ifdef ENSONIQ_USE_REGSISTRY 
    DWORD dwRet;
#endif
	ULONG ucIrq;
	ULONG ulIoBase;	 
	ULONG ulDMAOutPhysAddr;
	ULONG ulDMAInPhysAddr;


    FUNC_WPDD("PDD_AudioInitialize");

#ifdef ENSONIQ_USE_REGSISTRY 
    if ((dwRet = GetRegistryConfig((LPWSTR) dwIndex, &ucIrq, &ulIoBase))
        != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("WAVEDEV.DLL : GetRegistryConfig failed %d\r\n"),
            dwRet));
        return FALSE;
    }
#else
	ucIrq = 0;
	ulIoBase = 0;
#endif

	//
    // Create an ES1371 object to control the hardware 
	//
	if (pCES1371 == NULL) {
		pCES1371 = new CES1371();
		if (pCES1371 == NULL) {
			DEBUGMSG(ZONE_ERROR, (TEXT("Unable to allocate CES1371 object")));
			return FALSE;
		}

		// first thing: see if the device is really there
		if (!pCES1371->LocateAndMapHardware(ucIrq, ulIoBase)) {
			DEBUGMSG(1, (TEXT("PDD_AudioInitialize: failed to locate and map hardware \r\n")));
			delete pCES1371;
			pCES1371 = NULL;
			return FALSE;
		}
	}
	pCES1371->AddRef();

	//
	// Set the global master volume 
	//
	v_nVolume = ES1371_INITIALVOLUME;

	//
	// Alocate DMA buffers 
	//
	dma_buffer_size = MAXLEN_DMA_BUFFER;
	dma_page_size   = MAXLEN_DMA_BUFFER/AUDIO_DMA_NUMBER_PAGES;

#ifdef ENSONIQ_USE_PREALLOCATEDMEMORY
	dma_out_page[0] = (PUCHAR)VirtualAlloc(0,
									  dma_buffer_size * AUDIO_DMA_BUFFERS,
									  MEM_RESERVE,
									  PAGE_NOACCESS);
	if (!dma_out_page[0])  {
		ERRMSG("PDD_AudioInitialize: DMA Buffer Page Allocation Failed");
		return FALSE;
	}

	nRetVal = VirtualCopy((LPVOID) dma_out_page[0],
						 (LPVOID)(AUDIO_DMA_BUFFER_BASE_PA | 0x80000000),
						 dma_buffer_size * AUDIO_DMA_BUFFERS,
						 PAGE_READWRITE | PAGE_NOCACHE);
	if (nRetVal != TRUE )  {
		ERRMSG("PDD_AudioInitialize: pbBufPage Copy Failed");
		return FALSE;
	}

    ulDMAOutPhysAddr = AUDIO_DMA_BUFFER_BASE_PA;
	ulDMAInPhysAddr  = AUDIO_DMA_BUFFER_BASE_PA + dma_buffer_size

	//
	// Clear the buffers 
	//
	memset(dma_out_page[0],0,dma_buffer_size);
	memset(dma_in_page[0],0,dma_buffer_size);

	//
	// Setup dma_pages 
	//
	for ( i = 1; i < AUDIO_DMA_NUMBER_PAGES; i++ )
		dma_out_page[i] = dma_out_page[0] + (dma_page_size*i); 

	for ( i = AUDIO_DMA_NUMBER_PAGES; i < AUDIO_DMA_NUMBER_PAGES*2; i++ )
		dma_in_page[i]  = dma_out_page[0] + (dma_page_size*i); 
#else
	UINT cPages;
	PULONG aPages;

	//
	// Allocate a single paged 4k Ouput buffer
	//
	dma_out_page[0] = (PUCHAR)VirtualAlloc(0,
									  dma_buffer_size,
									  MEM_COMMIT ,
									  PAGE_READWRITE | PAGE_NOCACHE);
	if (!dma_out_page[0])  {
		ERRMSG("PDD_AudioInitialize: DMA Buffer Page Allocation Failed");
		return FALSE;
	}

    // let's first find the maximum # of pages that could be locked
    cPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(NULL, MAXLEN_DMA_BUFFER);

    // allocate an array for the maximum # of pages
    aPages = new ULONG[cPages];

    bRetVal = LockPages(dma_out_page[0], MAXLEN_DMA_BUFFER, aPages, LOCKFLAG_WRITE);
	if (bRetVal != TRUE )  {
		ERRMSG("PDD_AudioInitialize: pbBufPage Copy Failed");
		return FALSE;
	}

	// LockPages fills aPages[] with the PFNs (Page Frame Numbers?) of the locked pages.
	// We need to left-shift by a magic number to get the actual physical address.
	// Note that as of this writing, the PB online help incorrectly states that we should right-shift.
	// Note also that this code *assumes* that LockPages will lock down contiguous pages, which is not guaranteed
	// we should test that the pages are in fact contiguous and, if not, unlock them and try again,
	// probably with a single-page buffer.
	// kl  7/26/99

    ulDMAOutPhysAddr = aPages[0] << UserKInfo[KINX_PFN_SHIFT];
	DEBUGMSG(1, (TEXT("ENSONIQ1371: DMA Out Buffer Physical Address  %08x\r\n"), ulDMAOutPhysAddr));

	delete aPages;

	//
	// Allocate a single paged 4k Input buffer
	//
	dma_in_page[0] = (PUCHAR)VirtualAlloc(0,
									  dma_buffer_size,
									  MEM_COMMIT ,
									  PAGE_READWRITE | PAGE_NOCACHE);
	if (!dma_in_page[0])  {
	  ERRMSG("DMA Buffer Page Allocation Failed");
	  return FALSE;
	}

    // let's first find the maximum # of pages that could be locked
    cPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(NULL, MAXLEN_DMA_BUFFER);

    // allocate an array for the maximum # of pages
    aPages = new ULONG[cPages];

    bRetVal = LockPages(dma_in_page[0], MAXLEN_DMA_BUFFER, aPages, LOCKFLAG_WRITE);
	if (bRetVal != TRUE )  {
		ERRMSG("PDD_AudioEnable: pbBufPage Copy Failed");
		return FALSE;
	}

    ulDMAInPhysAddr = aPages[0] << UserKInfo[KINX_PFN_SHIFT];
	DEBUGMSG(1, (TEXT("ENSONIQ1371: DMA In  Buffer Physical Address  %08x\r\n"), ulDMAInPhysAddr));

	delete aPages;

	//
	// Clear the buffers 
	//
	memset(dma_out_page[0],0,dma_buffer_size);
	memset(dma_in_page[0],0,dma_buffer_size);

	//
	// Setup dma_pages 
	//
	for ( i = 1; i < AUDIO_DMA_NUMBER_PAGES; i++ )
		dma_out_page[i] = dma_out_page[0] + (dma_page_size*i); 

	for ( i = 1; i < AUDIO_DMA_NUMBER_PAGES; i++ )
		dma_in_page[i] = dma_in_page[0] + (dma_page_size*i); 

#endif


	//
	// Initialize the Ouput DMA chanel with the allocated buffer address
	// 
	pCES1371->InitDMAChannel( ES1371_DAC0, 
							  ulDMAOutPhysAddr, 
							  dma_buffer_size);

	//
	// Initialize the Input DMA chanel with the allocated buffer address
	// 
	pCES1371->InitDMAChannel( ES1371_ADC, 
							  ulDMAInPhysAddr, 
							  dma_buffer_size);

	return TRUE;
}


// -----------------------------------------------------------------------------
//								PDD_AudioMessage
// -----------------------------------------------------------------------------
DWORD
PDD_AudioMessage(
    UINT uMsg,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    //
    // Handle any custom WAVEIN or WAVEOUT messages here
    //
    return(MMSYSERR_NOTSUPPORTED);
}


// -----------------------------------------------------------------------------
//							PDD_AudioDeinitialize
// -----------------------------------------------------------------------------
VOID 
PDD_AudioDeinitialize(
    VOID
    )
{
    //
    // The driver is being unloaded. Deinitialize. Free up memory, etc.
    //

	if ( pCES1371 != NULL ) {
		if (0 == pCES1371->Release()) {
			pCES1371 = NULL;
		}
	}

    VirtualFree ((void*) dma_out_page[0], 0, MEM_RELEASE);
}


// -----------------------------------------------------------------------------
//								PDD_WaveProc
// -----------------------------------------------------------------------------
MMRESULT
PDD_WaveProc(
    WAPI_INOUT apidir,
    DWORD      dwCode,
    DWORD      dwParam1,
    DWORD      dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;


    switch (dwCode) {
        case WPDM_CLOSE:
            fInUse[apidir] = FALSE;
            break;

        case WPDM_CONTINUE:
            private_WaveOutContinue(apidir, (PWAVEHDR) dwParam1);
            break;

        case WPDM_GETDEVCAPS:
            mmRet = private_WaveGetDevCaps(apidir, (PVOID) dwParam1, (UINT) dwParam2);
            break;

        case WPDM_OPEN:
            mmRet = private_WaveOpen(apidir, (LPWAVEFORMATEX) dwParam1, (BOOL) dwParam2);
            break;

        case WPDM_STANDBY:
            private_WaveStandby (apidir);
            break;

        case WPDM_START:
            private_WaveOutStart(apidir, (PWAVEHDR) dwParam1);
            break;

        case WPDM_STOP:
            private_WaveOutStop(apidir);
            break;

        case WPDM_PAUSE:
            private_WaveOutPause(apidir);
            break;

        case WPDM_ENDOFDATA:
            private_WaveOutEndOfData(apidir);
            break;

        case WPDM_RESTART:
            private_WaveOutRestart(apidir,(PWAVEHDR) dwParam1);
            break;

        case WPDM_GETVOLUME:
            *((PULONG) dwParam1) = (v_nSetVolume & 0xF800F800);	// Return vaule as it would be in the card
            break;

        case WPDM_SETVOLUME:
			private_waveOutSetVolume(dwParam1);
            break;

        default :
            mmRet = MMSYSERR_NOTSUPPORTED;
            break;
    }

    return(mmRet);
}

#ifdef __cplusplus
}
#endif



