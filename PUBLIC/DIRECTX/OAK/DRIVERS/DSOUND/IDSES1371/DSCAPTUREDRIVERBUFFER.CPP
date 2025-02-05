//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-1999 Microsoft Corporation

Module Name:

	DsCaptureDriverBuffer.cpp

Abstract:

	Implementation file for the DsCaptureDriverBuffer class.


Revision History:

	a-gm 08/19/99 - Based on the new CE DirectSound driver model


*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
#include "includes.h"

//
// where we can find declaration for main class in this file:
//
//#include "DsCaptureDriverBuffer.h"
//
//
// where we can find declarations needed in this file:
//
//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////

// global variables pointing to pre-allocated memory
extern ULONG	g_pADC_PhysAddr;
extern ULONG	g_pADC_VirtAddr;

// A reference count to track DuplicateSoundBuffer
DWORD DsCaptureDriverBuffer::dwCloneRefCount = 0;


//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsCaptureDriverBuffer::DsCaptureDriverBuffer
(
	HRESULT &hr
)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::DsCaptureDriverBuffer\r\n")));

	hr = DS_OK;

	m_ulRefCount = 1;

	m_pNext = NULL;
	m_pPrevious = NULL;


	// Do initialization stuff here.
	m_bUsingPreAllocMemory = FALSE;
    m_pDMABuffer = 0;
	m_dwDMABufSize = 0;
	m_PhysAddr = 0;

	m_pActualBuffer = 0;
	m_dwActualBufSize = 0;

	m_pwfxFormat = (LPWAVEFORMATEX) new BYTE[sizeof(WAVEFORMATEX)];
	if(!m_pwfxFormat)
		hr = DSERR_OUTOFMEMORY;
	else
		memset(m_pwfxFormat, 0, sizeof(WAVEFORMATEX));

	m_channelindex = 2;
	m_bPrimary = FALSE;
	m_bIsCapturing = FALSE;
        m_bIsLooping = FALSE;

	m_pDsDriverCapture = NULL;

	m_dwWritePosition = 0;
	m_dwOldWritePosition = 0;

	m_CopyEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("__ES1371CAPTURECOPYEVENT__"));

	m_bExitRecorderThread = FALSE;
	m_dwRecordCopyChunkSize = 0;
	m_dwRecordCopyCursor = 0;

	m_dwCountOfNotifications = 0;
	m_pDsPositionNotify = NULL;
	m_dwNextNotification = 0;

	InitializeCriticalSection( &m_csDriverBuffer );
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsCaptureDriverBuffer::~DsCaptureDriverBuffer()
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::~DsCaptureDriverBuffer\r\n")));


	// Insert shutdown code here.

	if(m_bIsCapturing)
		Stop();

	if( m_pDsDriverCapture)
	{
		m_pDsDriverCapture->m_nActiveBuffers--;
		m_pDsDriverCapture->m_pCaptureBuffer = NULL;
	}

	if(m_pwfxFormat) 
		delete m_pwfxFormat;

	if(m_pActualBuffer) 
	{
		BOOL b = VirtualFree(m_pActualBuffer, 0, MEM_RELEASE);
		if(b)
			DEBUGMSG(ZONE_INFO, (TEXT("~DsCaptureDriverBuffer - VirtualFreed m_pActualBuffer")));
	}

	CloseHandle(m_CopyEvent);

	if(m_pDsPositionNotify) 
		delete m_pDsPositionNotify;

	if(! m_bUsingPreAllocMemory)
	{
		UnlockPages(m_pDMABuffer, m_dwDMABufSize);
		BOOL b = VirtualFree(m_pDMABuffer, 0, MEM_RELEASE);
		if(b)
			DEBUGMSG(ZONE_INFO,(TEXT("~DsCaptureDriverBuffer::Buf %d: Unlocked and freed ptr %x, size %x \r\n"), m_channelindex, m_pDMABuffer, m_dwDMABufSize));
	}

	DeleteCriticalSection( &m_csDriverBuffer );

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

QueryInterface is where DirectSound will ask us whether we support any 
additional capabilities for a buffer.  If we support other interfaces
(in other words, if we inherit from another abstract  C++ base class)
we should return a pointer to that interfaces here.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::QueryInterface (
	REFIID pIid,
	LPVOID *ppNew )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::QueryInterface\r\n")));
	

    DEBUGCHK(ppNew != NULL);


#ifdef PARAMETER_VALIDATIONS
	if( ! ppNew )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsCaptureDriverBuffer::QueryInterface -- invalid pointer\r\n")));
		return DSERR_INVALIDPARAM;
	}
#endif


	if( IsEqualIID(IID_IUnknown, pIid)
		|| IsEqualIID(IID_IDsCaptureDriverBuffer,pIid)
		)
	{
		
		// Increase RefCount.
		InterlockedIncrement( (LPLONG) &m_ulRefCount );


		// Cast ourselves to the type of base class requested.
		*ppNew = (IDsCaptureDriverBuffer *)this;
		return NO_ERROR;
	}
	else if ( IsEqualIID(IID_IDsDriverNotify, pIid) )
	{

		// Increase RefCount.
		InterlockedIncrement( (LPLONG) &m_ulRefCount );

		// Cast ourselves to the type of base class requested.
		*ppNew = (IDsDriverNotify *)(this);
		return NO_ERROR;
	}


	*ppNew = NULL;
	return DSERR_NOINTERFACE;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

AddRef and Release are how other clients of our component tell us that
it is no longer needed.  When the RefCount goes to zero, we know we are
not needed, and we will delete ourselves.


*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsCaptureDriverBuffer::AddRef(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsCaptureDriverBuffer::AddRef previous:%d\r\n"), m_ulRefCount));
	

	return InterlockedIncrement( (LPLONG) &m_ulRefCount );
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

AddRef and Release are how other clients of our component tell us that
it is no longer needed.  When the RefCount goes to zero, we know we are
not needed, and we will delete ourselves.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsCaptureDriverBuffer::Release(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsCaptureDriverBuffer::Release previous=%ld\r\n"), m_ulRefCount));
	

	ULONG ulRefs = InterlockedDecrement( (LPLONG) &m_ulRefCount );

	if( ulRefs == 0 )
	{
		DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsCaptureDriverBuffer::Release final on 0x%08x\n"), this ));
		delete this;
	}
	return ulRefs;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::Lock(
    /* [out] */ LPLPVOID ppAudio1,
    /* [out] */ LPDWORD pAudioBytes1,
    /* [out] */ LPLPVOID ppAudio2,
    /* [out] */ LPDWORD pAudioBytes2,
    /* [in] */ DWORD dwWriteCursor,
    /* [in] */ DWORD dwWriteBytes,
    /* [in] */ DWORD dsbLockFlags
	)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::Lock dwWriteCursor %x, dwWriteBytes %x dsbLockFlags %x, buf %d\r\n"), dwWriteCursor, dwWriteBytes, dsbLockFlags, m_channelindex));

	HRESULT hr = DS_OK;


	// Do stuff here to provide access to the actual sound buffer data as requested.
	// Return pointers to that data.

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	if(dsbLockFlags & DSCBLOCK_ENTIREBUFFER)
		dwWriteBytes = m_dwActualBufSize;

	// just return appropriate positions
	if(dwWriteBytes > m_dwActualBufSize)
		return DSERR_INVALIDPARAM;

	*ppAudio1 = m_pActualBuffer + dwWriteCursor;

	if(dwWriteBytes > m_dwActualBufSize - dwWriteCursor)
	{
		*pAudioBytes1 = m_dwActualBufSize - dwWriteCursor;
		if(ppAudio2)  *ppAudio2 = m_pActualBuffer;
		if(pAudioBytes2)  *pAudioBytes2 = dwWriteBytes - *pAudioBytes1;
	}
	else
	{
		*pAudioBytes1 = dwWriteBytes;

		if(ppAudio2)  *ppAudio2 = NULL;  
		if(pAudioBytes2)  *pAudioBytes2 = 0;
	}

	return hr;

}






//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::Unlock(
    /* [in] */ void *pAudio1,
    /* [in] */ DWORD dwAudioBytes1,
    /* [in] */ void *pAudio2,
    /* [in] */ DWORD dwAudioBytes2)
{

    DEBUGMSG(ZONE_INFO,(TEXT("DsCaptureDriverBuffer::Unlock %d\r\n"), m_channelindex));
//    DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::Unlock %d\r\n"), m_channelindex));
	

	HRESULT hr = DS_OK;

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// The client of this interface is finished with accessing the actual
	// data of the buffer.  

	// Do any tasks your hardware requires here.

	// For example, if your hardware supports on-board audio RAM, and this
	// is a static buffer, you may have provided a pointer to system RAM
	// in the Lock method, the client will have written some data to it, 
	// and at this point you can move that data to on-board RAM.
	
	// Nothing to do for the DMA based Ensoniq 1371


	return hr;

}




//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP DsCaptureDriverBuffer::Start(
    /* [in] */ DWORD dsbPlayFlags)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::Start\r\n")));
    DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::Start  flags=%x\r\n"),dsbPlayFlags));


	HRESULT hr = DS_OK;

	// Tell the device to start playing this buffer.
	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	if(m_bIsCapturing == TRUE)
		return DS_OK;

        if (dsbPlayFlags & DSCBSTART_LOOPING) {
	        m_bIsLooping = TRUE;
        }

	//	m_dwRecordCopyChunkSize = MIN(m_dwDMABufSize, m_dwActualBufSize)/NUM_INTERRUPTS_PER_CAPTUREBUFF;

	// We'll keep the chunk size at 100milliseconds min, this also determines the granularity of notification positions..
	m_dwRecordCopyChunkSize = MIN(MIN(m_dwDMABufSize/2, m_dwActualBufSize/2), m_pwfxFormat->nAvgBytesPerSec/10);

	m_numberofsamples = m_dwRecordCopyChunkSize/(((m_pwfxFormat->wBitsPerSample==8)?1:2) * m_pwfxFormat->nChannels);

	//set to LOOP mode
	m_pCES1371->HwRegRMW( ES1371_bSERCTL_OFF, (UCHAR)ES1371_SERCTL_R1LOOP, (UCHAR)0x00);

	m_pCES1371->SetDMAChannelBuffer( ES1371_ADC, m_dwDMABufSize, m_numberofsamples);

	m_dwNextNotification = 0;

	if(m_dwWritePosition >= m_dwActualBufSize)
		m_dwWritePosition = 0;

	// Start Recorder thread

	m_bExitRecorderThread = FALSE;
	ResetEvent(m_CopyEvent);

	DWORD dwThreadId=0;

	m_hRecorderThread = CreateThread(NULL, 0, RecorderThread, this, 0, &dwThreadId);
	if(m_hRecorderThread == NULL)
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsCaptureDriverBuffer::Start -- unable to create Recorder thread\r\n")));

		return DSERR_GENERIC;
	}

	m_bIsCapturing = TRUE;

	// Start the DMA channel
	m_pCES1371->StartDMAChannel( ES1371_ADC );

	return hr;
}

//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::Stop(void)
{
    //DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::Stop\r\n")));
	DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::Stop\r\n")));
	
	HRESULT hr = DS_OK;

	EnterCriticalSection(&m_csDriverBuffer);	//we can't use our smart class, because we are using SEH below

	if(m_bIsCapturing == FALSE)
	{
		LeaveCriticalSection(&m_csDriverBuffer);
		return DS_OK;
	}

	// Stop the dma buffer from recording
	m_pCES1371->StopDMAChannel( ES1371_ADC );

	__try
	{
	// Set last event if it applies
	if((m_dwCountOfNotifications > 0)
		&& m_pDsPositionNotify[m_dwCountOfNotifications-1].dwOffset == DSBPN_OFFSETSTOP)
		SetEvent(m_pDsPositionNotify[m_dwCountOfNotifications-1].hEventNotify);
	}
	__except(1 /*EXCEPTION_EXECUTE_HANDLER*/)
	{
		// happens when you kill the client process
		DEBUGMSG(ZONE_ERROR,(TEXT("DsCaptureDriverBuffer::Stop -- Error setting notification event\r\n")));
	}



	// Now Stop recorder thread

	m_bExitRecorderThread = TRUE;
	SetEvent(m_CopyEvent);

	if( WAIT_OBJECT_0 != WaitForSingleObject(m_hRecorderThread, INFINITE) )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsCaptureDriverBuffer::Close -- Error closing Recorder thread\r\n")));
		LeaveCriticalSection(&m_csDriverBuffer);
		return DSERR_GENERIC;
	}
	m_hRecorderThread = NULL;

	m_bIsCapturing = FALSE;
    m_bIsLooping = FALSE;

	LeaveCriticalSection(&m_csDriverBuffer);

	return hr;

}

//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::GetStatus( LPDWORD pdwStatus)
{
    DEBUGMSG(ZONE_ENTER, (TEXT("DsDriverBuffer::GetStatus\r\n")));

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	//DWORD dwPlay;

	if (m_bIsCapturing) {
		if (m_bIsLooping) {
			// we're playing and looping. It's easy
			*pdwStatus = DSCBSTATUS_CAPTURING | DSCBSTATUS_LOOPING;
		}
		else {
			*pdwStatus = DSCBSTATUS_CAPTURING;
		}
	}
	else {
		*pdwStatus = 0;
	}

	return DS_OK;
}


//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

  Notes:
Both the read pointer and the capture pointer represent
"first invalid" positions, meaning the client will want to
read up to BUT NOT INCLUDING the reported read position.
The read pointer trails the capture pointer and
marks the first "not necessarily valid" byte in the no-man's
land between the read cursor and the capture cursor
The idea is that the capture position represents where
the hardware is at, and the read position represents the data
we the client can actually get at. For example, if you're buffering
in from a smaller dma buffer, the the read cursor points to the
next byte you will copy in from the dma buffer.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::GetPosition( LPDWORD pdwCapture, LPDWORD pdwRead)
{
    DEBUGMSG(ZONE_ENTER, (TEXT("DsCaptureDriverBuffer::GetPosition\r\n")));

	HRESULT hr = DS_OK;

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// Return the current play and write positions in the buffer.
	// Play is what's currently coming out of the speaker.
	// Write is what's already committed (so place ahead of
	// which it is OK to write new data.

//	if(!pdwCapture || !pdwRead)
//		return DSERR_INVALIDPARAM;

	if (! m_bIsCapturing) {
		if(pdwCapture) *pdwCapture = 0;
		if(pdwRead) *pdwRead = 0; 
		return DS_OK;
	}

	// Capture position is where the hardware is at currently
	// Since we actually return a position on the actual buffer and not the dma buffer
	// ths is just teh write position + one chunk ahead.
	if(pdwCapture)
		*pdwCapture = m_dwWritePosition + m_dwRecordCopyChunkSize;

	// Read position should mark the end of valid data
	if(pdwRead) 
		*pdwRead = m_dwWritePosition;


    return hr;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::SetPosition(
    /* [in] */ DWORD dwPosition)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::SetPosition\r\n")));
	DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::SetPosition\r\n")));
	

	HRESULT hr = DS_OK;


	// Move play position of buffer to specified position.

	return DSERR_UNSUPPORTED;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::SetFormat(
    /* [in] */ LPWAVEFORMATEX pwfxFormatEx)
{

    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::SetFormat\r\n")));
	DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::SetFormat\r\n")));
	
	HRESULT hr = DS_OK;

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// Change format of existing buffer.

	//save format
	if(m_pwfxFormat) memcpy(m_pwfxFormat, pwfxFormatEx, sizeof(WAVEFORMATEX));


	m_pCES1371->SetDMAChannelFormat(ES1371_ADC,
		                      pwfxFormatEx->nChannels ,
			                  (pwfxFormatEx->wBitsPerSample==8)?0:1,
                              pwfxFormatEx->nSamplesPerSec);

    return hr;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::SetVolumePan(
    /* [in] */ PDSVOLUMEPAN pdsVolumePan)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::SetVolumePan\r\n")));
	DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::SetVolumePan\r\n")));

	HRESULT hr = DS_OK;
	CriticalSectionAcquisition csa(&m_csDriverBuffer);


	// Set the volume and pan for this buffer.
	//
	//	pdsVolumePan->lVolume;	in Hundredths of decibels  0 to 10,000
	//	pdsVolumePan->lPan;	-10,000 to +10,000


	return DSERR_CONTROLUNAVAIL;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::SetFrequency(
    /* [in] */ DWORD dwFrequency)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::SetFrequency\r\n")));
	DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::SetFrequency\r\n")));
	
	HRESULT hr = DS_OK;


	// Do stuff to set frequency here.


	return DSERR_CONTROLUNAVAIL;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

SetNotificationPositions is a method on the IDsDriverNotify interface.
If your buffer supports this interfaces (in other words, if your hardware
provides support for alerting that play position has passed a certain point)
you need to implement this method.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsCaptureDriverBuffer::SetNotificationPositions(
	/* [in] */ DWORD dwCountOfNotifications,
	/* [in] */ LPCDSBPOSITIONNOTIFY lpcdsbpn)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::SetNotificationPositions\r\n")));
    DEBUGMSG(1,(TEXT("DsCaptureDriverBuffer::SetNotificationPositions\r\n")));
	
	HRESULT hr = DS_OK;

	// Do whatever you need to do to store the array of positions and the 
	// associated event for each position.

	// Set the hardware up to notify that play has passed the indicated
	// positions.  When the buffer's play position has passes a point,
	// call SetEvent on the associated event that was passed in here.

	if( dwCountOfNotifications != 0 && lpcdsbpn == NULL)
		return DSERR_INVALIDPARAM;

	m_dwCountOfNotifications = dwCountOfNotifications;
	m_pDsPositionNotify = (DSBPOSITIONNOTIFY *)lpcdsbpn;
	Stop();

	return hr;
}


BOOL DsCaptureDriverBuffer::Allocate(DWORD dwBytesToAllocate)
{
	DEBUGMSG(ZONE_ENTER,(TEXT("DsCaptureDriverBuffer::Allocate\r\n")));

	ASSERT( ! m_pDMABuffer );
	ASSERT( ! m_dwDMABufSize );


	// We'll Allocate two buffers
	// 1. a DMA buffer minimum 2 pages in length and locked in physical memory
	// 2. the actual buffer of the desired length
	// We'll double buffer the first one and copy to the second one. 
	// This is needed because directsound capture support currently can't 
	// handle buffers whose length is different than the one requested
	// (the size returned thru pdwcbBufferSize should be same as requested)
	// Also this will enable us to implement SetNotificationPosition neatly

	// 1. DMA buffer

	m_dwDMABufSize = PRE_ALLOC_BUFFER_SIZE_ADC;

	if(g_pADC_VirtAddr)	// If pre-allocated
	{
		m_PhysAddr = g_pADC_PhysAddr;
		m_pDMABuffer = (PUCHAR) g_pADC_VirtAddr;
		m_bUsingPreAllocMemory = TRUE;
	}
	else
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);

		// Try to allocate full size ie, PRE_ALLOC_BUFFER_SIZE_ADC
		if(!ReservePhysicalMem(m_dwDMABufSize, &m_PhysAddr, (ULONG*) &m_pDMABuffer))
		{
			// if failed, try to allocate 2 pages
			m_dwDMABufSize = 2 * si.dwPageSize;
			if(! ReservePhysicalMem(m_dwDMABufSize, &m_PhysAddr, (ULONG*) &m_pDMABuffer))
				return FALSE;
		}
	}

	// 2. Actual buffer

	m_dwActualBufSize = dwBytesToAllocate;

	m_pActualBuffer = (LPBYTE)VirtualAlloc(0,
								  m_dwActualBufSize,
								  MEM_COMMIT ,
								  PAGE_READWRITE | PAGE_NOCACHE);
	if (!m_pActualBuffer)  {
		DEBUGMSG(1, (TEXT("DsCaptureDriverBuffer::Allocate -- unable to allocate memory for buffer object")));
		return FALSE;
	}

	// Clear the buffers
	memset(m_pDMABuffer, 0, m_dwDMABufSize);	// DMA buffer
	memset(m_pActualBuffer, 0, m_dwActualBufSize);	// Actual buffer

	//After we allocate memory, we need to associate the buffer with an available dma channel

	m_pCES1371->InitDMAChannel(ES1371_ADC,		// Capture Channel
							  m_PhysAddr,		// Physical address of DMA buffer
							  m_dwDMABufSize);		// Size of DMA buffer


	DEBUGMSG(ZONE_INFO, (TEXT("DsCaptureDriverBuffer:Allocate --DMABuff buf %d: Buffer size %x and locked at phys addr %x \r\n"),m_channelindex, m_dwDMABufSize, m_PhysAddr));
	DEBUGMSG(ZONE_INFO, (TEXT("DsCaptureDriverBuffer:Allocate --ActualBuff buf %d: Buffer size %x \r\n"),m_channelindex, m_dwActualBufSize));

	m_pDsDriverCapture->m_nActiveBuffers++;

	return TRUE;
}


VOID DsCaptureDriverBuffer::ProcessInterrupt()
{
	// Reload interrupt counter
    m_pCES1371->HwRegRMW(ES1371_wADCIC_OFF, (USHORT)0xffff, (USHORT)(m_numberofsamples - 1));

	// Signal RecorderThread to copy data
	SetEvent(m_CopyEvent);
}


VOID DsCaptureDriverBuffer::ProcessCopyEvent()
{
	// Copy appropriate portion of DMA buffer to Actual buffer
	DWORD i=0;

	// COPY
	// FROM dma buffer:    m_pDMABuffer + m_dwRecordCopyCursor 
	// TO	actual buffer: m_pActualBuffer + m_dwWritePosition
	// SIZE m_dwRecordCopyChunkSize
	// NOTE: m_pDMABuffer and m_pActualBuffer are of type LPBYTE
	// also need to handle wrap around conditions neatly

	BYTE *pSrc = m_pDMABuffer + m_dwRecordCopyCursor;
	BYTE *pDst = m_pActualBuffer + m_dwWritePosition;
	if( m_dwDMABufSize - m_dwRecordCopyCursor >= m_dwRecordCopyChunkSize)
	{
		if(m_dwActualBufSize - m_dwWritePosition >= m_dwRecordCopyChunkSize)
			memcpy(pDst, pSrc, m_dwRecordCopyChunkSize);
		else
		{
			memcpy(pDst, pSrc, m_dwActualBufSize - m_dwWritePosition);
			memcpy(m_pActualBuffer, pSrc + m_dwActualBufSize - m_dwWritePosition,
				m_dwRecordCopyChunkSize - m_dwActualBufSize + m_dwWritePosition);
		}
	}
	else
	{
	//	memcpy(pDst, pSrc, m_dwRecordCopyChunkSize);
		if(m_dwActualBufSize - m_dwWritePosition >= m_dwRecordCopyChunkSize)
		{
			memcpy(pDst, pSrc, m_dwDMABufSize - m_dwRecordCopyCursor);
			memcpy(pDst + m_dwDMABufSize - m_dwRecordCopyCursor, m_pDMABuffer,
				m_dwRecordCopyChunkSize - m_dwDMABufSize + m_dwRecordCopyCursor);
		}
		else
		{
			if(m_dwDMABufSize - m_dwRecordCopyCursor >= m_dwActualBufSize - m_dwWritePosition)
			{
				memcpy(pDst, pSrc, m_dwActualBufSize - m_dwWritePosition);
				memcpy(m_pActualBuffer, pSrc + m_dwActualBufSize - m_dwWritePosition,
					m_dwDMABufSize - (m_dwRecordCopyCursor + m_dwActualBufSize - m_dwWritePosition));
				memcpy(m_pActualBuffer + (m_dwRecordCopyCursor + m_dwActualBufSize - m_dwWritePosition),
					m_pDMABuffer,
					m_dwRecordCopyChunkSize + m_dwRecordCopyCursor - m_dwDMABufSize);
			}
			else
			{
				memcpy(pDst, pSrc, m_dwDMABufSize - m_dwRecordCopyCursor);
				memcpy(pDst + m_dwDMABufSize - m_dwRecordCopyCursor, m_pDMABuffer, 
					m_dwActualBufSize - (m_dwWritePosition + m_dwDMABufSize - m_dwRecordCopyCursor));
				memcpy(m_pActualBuffer, m_pDMABuffer + (m_dwWritePosition + m_dwDMABufSize - m_dwRecordCopyCursor),
					m_dwRecordCopyChunkSize + m_dwWritePosition - m_dwActualBufSize);
			}
		}
	}

	// Update Actual buffer's cursor to next position
	m_dwOldWritePosition = m_dwWritePosition;
	m_dwWritePosition	+= m_dwRecordCopyChunkSize;
	m_dwWritePosition %= m_dwActualBufSize;

	// Update DMA buffer's copy cursor to next position
	m_dwRecordCopyCursor = (m_dwRecordCopyCursor + m_dwRecordCopyChunkSize) % m_dwDMABufSize;

	__try
	{
	// Check notification positions and notify
	for(i = 0; i< m_dwCountOfNotifications; i++)
	{
		DWORD dwNotifyPosition = m_pDsPositionNotify[m_dwNextNotification].dwOffset;

		if( (m_dwWritePosition > dwNotifyPosition && m_dwOldWritePosition <= dwNotifyPosition)
			||
			(	(m_dwOldWritePosition > m_dwWritePosition) 
				&& (dwNotifyPosition >= m_dwOldWritePosition || m_dwWritePosition > dwNotifyPosition)
			)
		  )
		{
			//set event
			SetEvent(m_pDsPositionNotify[m_dwNextNotification].hEventNotify);
			//DEBUGMSG(1, (TEXT("IDSES1371:Capture: SetEvent No. %d of %d\r\n"), m_dwNextNotification, m_dwCountOfNotifications));
			m_dwNextNotification++;
			m_dwNextNotification %= m_dwCountOfNotifications;
			if(m_pDsPositionNotify[m_dwNextNotification].dwOffset == DSBPN_OFFSETSTOP)
				m_dwNextNotification = 0;
		}
		else
			break;
	}
	}
	__except(1 /*EXCEPTION_EXECUTE_HANDLER*/)
	{
		// happens when you kill the client process
		DEBUGMSG(ZONE_ERROR,(TEXT("DsCaptureDriverBuffer::ProcessCopyEvent -- Error setting notification event\r\n")));
		m_dwCountOfNotifications = 0;
		m_pDsPositionNotify = NULL;
	}

	// If we've reached the end and LOOP is not specified, we have to stop the buffer
	if(!m_bIsLooping)
	{
		if(m_dwWritePosition <= m_dwOldWritePosition)
		{
			m_dwWritePosition = m_dwActualBufSize + 1; // to indicate completion, set position to buffer size +1

			// Set last event if it applies
			if((m_dwCountOfNotifications > 0)
				&& m_pDsPositionNotify[m_dwCountOfNotifications-1].dwOffset == DSBPN_OFFSETSTOP)
				SetEvent(m_pDsPositionNotify[m_dwCountOfNotifications-1].hEventNotify);
			m_bExitRecorderThread = TRUE;
			SetEvent(m_CopyEvent);
		}
	}

}



static DWORD WINAPI RecorderThread(LPVOID lpParameter)
{
	//Initialize

	DsCaptureDriverBuffer *pThis = (DsCaptureDriverBuffer *)lpParameter;

	// --for SetNotificationPosition
	// We need to have the right ACL's to be able to set the events
	// which were created in another process.
	// Save the old permissions.
	DWORD dwSavedPermissions = GetCurrentPermissions();
	DWORD dwNewPermissions = SetProcPermissions( DWORD (-1) );
	// I think the platform docs are bogus here -- SetProcPermissions
	// seems to be returning the previous permissions, not the new permissions.
	// ASSERT( dwNewPermissions == DWORD (-1) );
	ASSERT( GetCurrentPermissions() == DWORD (-1) );

    BOOL fResult = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	if(!fResult)
		DEBUGMSG(1, (TEXT("CES1371::RecorderThread - Warning - thread priority couldn't be changed to HIGHEST\r\n")));

	DWORD dwRet=0;
	BOOL  bKeepLooping = TRUE;

	pThis->m_dwWritePosition = 0;
	while(bKeepLooping)
	{
		// DEBUGMSG(1, (TEXT("IDSES1371:Capture: Recorder thread looping..\r\n")));

		dwRet = WaitForSingleObject(pThis->m_CopyEvent, INFINITE);

		if(pThis->m_bExitRecorderThread == TRUE)
			break;

		switch(dwRet)
		{
		case WAIT_OBJECT_0:
			pThis->ProcessCopyEvent();
			break;

		case WAIT_ABANDONED:
			DEBUGMSG(1, (TEXT("IDSES1371:Capture: Recorder thread wait Abandoned..\r\n")));
		case WAIT_TIMEOUT:
			DEBUGMSG(1, (TEXT("IDSES1371:Capture: Recorder thread wait Timedout..\r\n")));
		case WAIT_FAILED:
			DEBUGMSG(1, (TEXT("IDSES1371:Capture: Recorder thread wait failed..\r\n")));
			bKeepLooping = FALSE;
			break;
		default:
			bKeepLooping = FALSE;
		}
	}

	// Restore the old permissions.
	dwNewPermissions = SetProcPermissions( dwSavedPermissions );
	// I think the platform docs are bogus here -- SetProcPermissions
	// seems to be returning the previous permissions, not the new permissions.
	//	ASSERT( dwNewPermissions == dwSavedPermissions );
	ASSERT( GetCurrentPermissions() == dwSavedPermissions  );

	pThis->m_bIsCapturing = FALSE;
    pThis->m_bIsLooping = FALSE;
	pThis->m_pCES1371->StopDMAChannel( ES1371_ADC );
	DEBUGMSG(1, (TEXT("IDSES1371:Capture: Recorder thread exiting..\r\n")));

	return 0;
}

