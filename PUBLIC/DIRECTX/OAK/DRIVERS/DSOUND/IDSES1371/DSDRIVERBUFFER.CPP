//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-1999 Microsoft Corporation

Module Name:

	DsDriverBuffer.cpp

Abstract:

	Implementation file for the DsDriverBuffer class.

Revision History:

	a-gm 08/19/99 - Based on the new CE DirectSound driver model
	kl 11/4/99
		- Implemented GetStatus
		- Modified GetPosition so it can alway tell the truth about the
		  play position instead of fugding to indicate stopped
		- Tidied up DEBUGMSG usage
	kl 11/5/99
		- In GetStatus, the comparison of dwPlay to m_dwOldPlayPos was 
		  backwards - we know we're stopped when the play cursor moves backwards
		  (presumably back to zero).
	kl 11/7/99
		- Modified SetFrequency to return DS_OK if frequency is native format
		- Modified SetPosition to return DS_OK if position is 0
		- Modified SetVolumePan to return DS_OK unconditionally
		  These changes were minimal requirements to get the HW secondary buffer
		  running under DSHOW. The DSHOW filter doesn't handle failure of the
		  above functions very gracefully...
		  One of these days we can fully implement these, but this is better than nothing.
	kl 11/18/99
		- Generalized the notification position implementation, using PLAY_CHUNKS, to
		  support notification positions at arbitrary offsets with complete precision


*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
#include "includes.h"
#include <math.h>
//
// where we can find declaration for main class in this file:
//

//
//
// where we can find declarations needed in this file:
//
//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////

// global variables pointing to pre-allocated memory
extern ULONG	g_pDAC0_PhysAddr;
extern ULONG	g_pDAC0_VirtAddr;
extern ULONG	g_pDAC1_PhysAddr;
extern ULONG	g_pDAC1_VirtAddr;


extern USHORT	g_VolLeft;
extern USHORT	g_VolRight;


// A reference count to track DuplicateSoundBuffer
DWORD DsDriverBuffer::dwCloneRefCount = 0;

//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsDriverBuffer::DsDriverBuffer
(
	HRESULT &hr
)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::DsDriverBuffer\r\n")));

	hr = DS_OK;

	m_ulRefCount = 1;

	m_pNext = NULL;
	m_pPrevious = NULL;


	// Do initialization stuff here.
    m_pBits = 0;
	m_dwSize = 0;
	m_dwWritePosition = 0;
//	m_dwCurrentChunkSize = 0;
	m_channelindex = 0xff; //will be set to correct value by DsDriver:CreateSoundBuffer;

	m_pwfxFormat = (LPWAVEFORMATEX) new BYTE[sizeof(WAVEFORMATEX)];
	if(!m_pwfxFormat)
		hr = DSERR_OUTOFMEMORY;
	else
		memset(m_pwfxFormat, 0, sizeof(WAVEFORMATEX));

	m_PhysAddr = 0;

	m_pDsDriver = NULL;
	m_dsbPlayFlags = 0xFFFFFFFF;
	m_fIsPlaying = FALSE;
	m_dwOldPlayPos = 0;
	m_bPrimary = FALSE;

	m_bUsingPreAllocMemory = FALSE;
	InitializeCriticalSection( &m_csDriverBuffer );

	m_VolumePan.lVolume = m_VolumePan.lPan = 0;

	m_dwNumberOfChunks = 0;
	m_dwCurrentChunk = 0;
	m_pPlayChunks = NULL;
	m_hEventStop = NULL;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsDriverBuffer::~DsDriverBuffer()
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::~DsDriverBuffer\r\n")));

	// Insert shutdown code here.
	if(m_fIsPlaying == TRUE)
		Stop();

	// Handle duplicate buffer details..
	// if no duplicates exist for m_pBits, release it
	//BUG BUG... since DsDriverBuffer::dwCloneRefCount is shared, we need to make it thread safe.
	if(DsDriverBuffer::dwCloneRefCount == 0)
	{
		if(m_pBits)
		{
			if(! m_bUsingPreAllocMemory)
			{
				UnlockPages(m_pBits, m_dwSize);
				VirtualFree(m_pBits, 0, MEM_RELEASE);

				DEBUGMSG(ZONE_INFO,(TEXT("~DsDriverBuffer::Buf %d: Unlocked and freed ptr %x, size %x \r\n"), m_channelindex, m_pBits, m_dwSize));
			}
		}
	}
	else
		DsDriverBuffer::dwCloneRefCount --;

	if(m_pPlayChunks)
		delete [] m_pPlayChunks;

	if(m_pwfxFormat) 
		delete m_pwfxFormat;

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
STDMETHODIMP DsDriverBuffer::QueryInterface (
	REFIID pIid,
	LPVOID *ppNew )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::QueryInterface\r\n")));


    DEBUGCHK(ppNew != NULL);


#ifdef PARAMETER_VALIDATIONS
	if( ! ppNew )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverBuffer::QueryInterface -- invalid pointer\r\n")));
		return DSERR_INVALIDPARAM;
	}
#endif


	if( IsEqualIID(IID_IUnknown, pIid)
		|| IsEqualIID(IID_IDsDriverBuffer,pIid)
		)
	{

		// Increase RefCount.
		InterlockedIncrement( (LPLONG) &m_ulRefCount );


		// Cast ourselves to the type of base class requested.
		*ppNew = (IDsDriverBuffer *)this;
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
STDMETHODIMP_(ULONG) DsDriverBuffer::AddRef(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriverBuffer::AddRef previous:%d\r\n"), m_ulRefCount));
	

	return InterlockedIncrement( (LPLONG) &m_ulRefCount );
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

AddRef and Release are how other clients of our component tell us that
it is no longer needed.  When the RefCount goes to zero, we know we are
not needed, and we will delete ourselves.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsDriverBuffer::Release(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriverBuffer::Release previous=%ld\r\n"), m_ulRefCount));
	

	ULONG ulRefs = InterlockedDecrement( (LPLONG) &m_ulRefCount );

	if( ulRefs == 0 )
	{
		DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriverBuffer::Release final on 0x%08x\n"), this ));

		if(m_channelindex < 2) 
			// Unassociate from DsDriver's list of buffers
			m_pDsDriver->SetBuffer(m_channelindex, NULL, FALSE);

		delete this;
	}
	return ulRefs;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::Lock(
    /* [out] */ LPLPVOID ppAudio1,
    /* [out] */ LPDWORD pAudioBytes1,
    /* [out] */ LPLPVOID ppAudio2,
    /* [out] */ LPDWORD pAudioBytes2,
    /* [in] */ DWORD dwWriteCursor,
    /* [in] */ DWORD dwWriteBytes,
    /* [in] */ DWORD dsbLockFlags
	)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::Lock dwWriteCursor %x, dwWriteBytes %x, buf %d\r\n"), dwWriteCursor, dwWriteBytes, m_channelindex));
	

	HRESULT hr = DS_OK;


	// Do stuff here to provide access to the actual sound buffer data as requested.
	// Return pointers to that data.

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// Buffer is already locked (when allocated). So we'll just return appropriate positions


	if(dwWriteBytes > m_dwSize)
		return DSERR_INVALIDPARAM;

	*ppAudio1 = m_pBits + dwWriteCursor;

	if(dwWriteBytes > m_dwSize - dwWriteCursor)
	{
		*pAudioBytes1 = m_dwSize - dwWriteCursor;
		if(ppAudio2)  *ppAudio2 = m_pBits;
		if(pAudioBytes2)  *pAudioBytes2 = dwWriteBytes - *pAudioBytes1;

		m_dwWritePosition = *pAudioBytes2;
	}
	else
	{
		*pAudioBytes1 = dwWriteBytes;

		if(ppAudio2)  *ppAudio2 = NULL;  
		if(pAudioBytes2)  *pAudioBytes2 = 0;

		m_dwWritePosition = dwWriteCursor + *pAudioBytes1;
	}

	if(m_dwWritePosition >= m_dwSize)
		m_dwWritePosition = 0;
//	m_dwCurrentChunkSize = dwWriteBytes;

	return hr;

}






//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::Unlock(
    /* [in] */ void *pAudio1,
    /* [in] */ DWORD dwAudioBytes1,
    /* [in] */ void *pAudio2,
    /* [in] */ DWORD dwAudioBytes2)
{

	DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::Unlock pAudio1 %x, pAudio2 %x, buf %d\r\n"), pAudio1, pAudio2, m_channelindex));
	

	HRESULT hr = DS_OK;

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// The client of this interface is finished with accessing the actual
	// data of the buffer.  

	// Do any tasks your hardware requires here.

	// For example, if your hardware supports on-board audio RAM, and this
	// is a static buffer, you may have provided a pointer to system RAM
	// in the Lock method, the client will have written some data to it, 
	// and at this point you can move that data to on-board RAM.
	
	// ### Nothing to do for the DMA based Ensoniq 1371, just update positions
//	m_dwCurrentChunkSize = dwAudioBytes1 + dwAudioBytes2;

	if(pAudio2)
		m_dwWritePosition = dwAudioBytes2;
	else
		m_dwWritePosition = ((LPBYTE)pAudio1 - m_pBits) + dwAudioBytes1;

	if(m_dwWritePosition >= m_dwSize)
		m_dwWritePosition = 0;

	return hr;

}




//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::Play(
    /* [in] */ DWORD reserved1,
    /* [in] */ DWORD reserved2,
    /* [in] */ DWORD dsbPlayFlags)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::Play\r\n")));


	HRESULT hr = S_OK;

	// Tell the device to start playing this buffer.
	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// If play flag has changed from previous call..update LOOP property
	if(m_dsbPlayFlags != dsbPlayFlags)
	{
		switch(m_channelindex )
		{
		case 0:
			if(!(dsbPlayFlags & DSBPLAY_LOOPING))
				m_pCES1371->HwRegRMW( ES1371_bSERCTL_OFF, (UCHAR)ES1371_SERCTL_P0LOOP, (UCHAR)ES1371_SERCTL_P0LOOP);
			else
				m_pCES1371->HwRegRMW( ES1371_bSERCTL_OFF, (UCHAR)ES1371_SERCTL_P0LOOP, (UCHAR)0x00);

			break;
		case 1:
			if(!(dsbPlayFlags & DSBPLAY_LOOPING))
				m_pCES1371->HwRegRMW( ES1371_bSERCTL_OFF, (UCHAR)ES1371_SERCTL_P1LOOP, (UCHAR)ES1371_SERCTL_P1LOOP);
			else
				m_pCES1371->HwRegRMW( ES1371_bSERCTL_OFF, (UCHAR)ES1371_SERCTL_P1LOOP, (UCHAR)0x00);

			break;
		default:
			return DSERR_GENERIC;
		}
	}
	// save flags
	m_dsbPlayFlags = dsbPlayFlags;

	// If already playing, return OK
	// Special case -- for Single shot buffers, we need to restart
	if(m_fIsPlaying && (m_dsbPlayFlags & DSBPLAY_LOOPING) )
		return hr;

	m_fIsPlaying = TRUE;

	// IDsDriverBuffer::Play does not support SetPosition other than 0.
	// several things would have to get generalized here... including figuring out which
	// chunk we're starting on and what the initial count would be.

	DWORD dwSampleCount1;
	DWORD dwSampleCount2;
	if (m_dwNumberOfChunks > 0) {
		m_dwCurrentChunk = 0;
		DWORD dwNextChunk = (m_dwCurrentChunk + 1) % m_dwNumberOfChunks;
		dwSampleCount1 = m_pPlayChunks[m_dwCurrentChunk].dwSampleCount;
		dwSampleCount2 = m_pPlayChunks[dwNextChunk].dwSampleCount;
	}
	else {
		// no chunks, or rather, just one big one - the size of the buffer
		dwSampleCount1 = dwSampleCount2 = m_dwSize/m_pwfxFormat->nBlockAlign;
	}

	switch(m_channelindex )
	{
	case 0:
		m_pCES1371->SetDMAChannelBuffer( ES1371_DAC0, m_dwSize, (USHORT) dwSampleCount1);
		m_pCES1371->StartDMAChannel( ES1371_DAC0 );
		// once it's up and playing the first chunk, arm the counter for the next one
		m_pCES1371->HwRegRMW(ES1371_wDAC0IC_OFF, (USHORT)0xffff, (USHORT) dwSampleCount2);
		break;
	case 1:
		m_pCES1371->SetDMAChannelBuffer( ES1371_DAC1, m_dwSize, (USHORT) dwSampleCount1 );
		m_pCES1371->StartDMAChannel( ES1371_DAC1 );
		// once it's up and playing the first chunk, arm the counter for the next one
		m_pCES1371->HwRegRMW(ES1371_wDAC1IC_OFF, (USHORT)0xffff, (USHORT) dwSampleCount2 );
		break;
	default:
		return DSERR_GENERIC;
	}

	DEBUGMSG(ZONE_INFO,(TEXT("IDSES1371: Playing buf %d... \r\n"), m_channelindex ));
	DEBUGMSG(ZONE_INFO,(TEXT("IDSES1371: GetTickCount  %x \r\n"), GetTickCount()));
	return hr;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::Stop(void)
{
	DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::Stop -- buf %d\r\n"), m_channelindex));
	
	HRESULT hr = DS_OK;

	if(m_fIsPlaying == FALSE)
		return hr;

	// Stop the buffer from playing here.

	switch(m_channelindex )
	{
	case 0:
		m_pCES1371->StopDMAChannel( ES1371_DAC0 );
		break;
	case 1:
		m_pCES1371->StopDMAChannel( ES1371_DAC1 );
		break;
	default:
		return DSERR_GENERIC;
	}

	m_dwOldPlayPos = 0;

	m_fIsPlaying = FALSE;

	// --for SetEvent
	// We need to have the right ACL's to be able to set the events
	// which were created in another process.
	// Save the old permissions.
	DWORD dwSavedPermissions = GetCurrentPermissions();
	SetProcPermissions( DWORD (-1) );

	__try
	{
		// set the Stop notification, if one is registered
		if (m_hEventStop != NULL) {
			SetEvent(m_hEventStop);
		}
	}
	__except(1 /*EXCEPTION_EXECUTE_HANDLER*/)
	{
		// happens when you kill the client process
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverBuffer::Stop -- Error setting notification event\r\n")));
	}

	// Restore the old permissions.
	SetProcPermissions( dwSavedPermissions );

	return hr;

}

//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::GetStatus( LPDWORD pdwStatus)
{
    DEBUGMSG(ZONE_ENTER, (TEXT("DsDriverBuffer::GetStatus\r\n")));

	if (m_fIsPlaying) {
		if (m_dsbPlayFlags & DSBPLAY_LOOPING) {
			*pdwStatus = DSBSTATUS_PLAYING | DSBSTATUS_LOOPING;
		}
		else {
			*pdwStatus = DSBSTATUS_PLAYING;
		}
	}
	else {
		*pdwStatus = 0;
	}

	return DS_OK;
}


//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::GetPosition( LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    DEBUGMSG(ZONE_ENTER, (TEXT("DsDriverBuffer::GetPosition\r\n")));

	HRESULT hr = DS_OK;

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// Return the current play and write positions in the buffer.
	// Play is what's currently coming out of the speaker.
	// Write is what's already committed (so place ahead of
	// which it is OK to write new data.

	if(!pdwPlay || !pdwWrite)
		return DSERR_INVALIDPARAM;

	if (! m_fIsPlaying) {
		*pdwPlay = *pdwWrite = 0;
		return hr;
	}

	switch(m_channelindex )
	{
	case 0:	
		*pdwPlay = m_pCES1371->GetDMAPosition(ES1371_DAC0) ;
		break;
	case 1:
		*pdwPlay = m_pCES1371->GetDMAPosition(ES1371_DAC1) ;
		break;
	default:
		return DSERR_GENERIC;
	}

	// Since the card DMA reads data 8 long words(32 bytes) at a time, the write position 
	// will be the next block of 32 bytes that is going to be transferred. For more safety,
	// we'll keep the write position another 32 byte chunk ahead.
	*pdwWrite = (*pdwPlay + (32 - *pdwPlay % 32) + 32) % m_dwSize;

	//save old value of play position
	m_dwOldPlayPos = *pdwPlay;

	//DEBUGMSG(1, (TEXT("DsDriverBuffer::GetPosition--buf %d--size %x, write %x, play %x\r\n"), m_channelindex, m_dwSize, *pdwWrite, *pdwPlay));

    return hr;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::SetPosition(
    /* [in] */ DWORD dwPosition)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::SetPosition\r\n")));


	HRESULT hr = DS_OK;


	// Move play position of buffer to specified position.
	if (dwPosition == 0) {
		// all we really support is setting the position to 0
		return DS_OK;
	}

	return DSERR_UNSUPPORTED;

	//return hr;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::SetFormat(
    /* [in] */ LPWAVEFORMATEX pwfxFormatEx)
{

    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::SetFormat\r\n")));
	
	HRESULT hr = DS_OK;

	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// Change format of existing buffer.

	//save format
	if(m_pwfxFormat) memcpy(m_pwfxFormat, pwfxFormatEx, sizeof(WAVEFORMATEX));

	switch(m_channelindex)
	{
	case 0:
		m_pCES1371->SetDMAChannelFormat(ES1371_DAC0,
                              pwfxFormatEx->nChannels ,
                              (pwfxFormatEx->wBitsPerSample==8)?0:1,
                              pwfxFormatEx->nSamplesPerSec);
		break;
	case 1:
		m_pCES1371->SetDMAChannelFormat(ES1371_DAC1,
                              pwfxFormatEx->nChannels ,
                              (pwfxFormatEx->wBitsPerSample==8)?0:1,
                              pwfxFormatEx->nSamplesPerSec);
		break;
	default:
		return DSERR_GENERIC;
	}


    return hr;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::SetVolumePan(
    /* [in] */ PDSVOLUMEPAN pdsVolumePan)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::SetVolumePan\r\n")));

	HRESULT hr = DS_OK;

	if(NULL == pdsVolumePan)
		return DSERR_INVALIDPARAM;
	
	CriticalSectionAcquisition csa(&m_csDriverBuffer);

	// Set the volume and pan for this buffer.

	//	pdsVolumePan->lVolume;	in Hundredths of decibels  0 to -10,000
	//	pdsVolumePan->lPan;	-10,000 to +10,000
	//  Above decibel values to be converted to linear values

	// --------------------------------------------------------------------------
	// Decibel fundas:
	//   Number of decibels = 10 x log10( level / reference level )
	// 
	//   Our reference level is g_VolLeft (ES1371_INITIALVOLUME >> 8) for left channel
	//   and g_VolRight (ES1371_INITIALVOLUME & 0xff) for right channel

	// So given decibels and the reference level we can find the level by
	//    level =  10^(dBs/10) * refLevel

	// We'll use FP math for now, we'll worry about efficiency later
	// --------------------------------------------------------------------------

    double dbPan;
	double dbVolume;
	USHORT usLeftVal, usRightVal;

	dbVolume = (pdsVolumePan->lVolume)/100.0;
	dbPan = (pdsVolumePan->lPan)/100.0;

	if(dbPan > 0){
		//right channel at full vol, and left channel attenuated

		usLeftVal = (USHORT) (pow(10, (dbVolume - dbPan)/10.0) * g_VolLeft);
		usRightVal = (USHORT) (pow(10, (dbVolume)/10.0) * g_VolRight);

	}
	else{
		//left channel at full vol, and right channel attenuated

		usRightVal = (USHORT) (pow(10, (dbVolume + dbPan)/10.0) * g_VolRight);
		usLeftVal = (USHORT) (pow(10, (dbVolume)/10.0) * g_VolLeft);
	}

	switch(m_channelindex)
	{
	case 0:
		//For the primary buffer we need to change the volume of the whole card

		// write master volume
		m_pCES1371->WriteCodecRegister( AC97_MASTER_VOL_STEREO, (usLeftVal<<8) | usRightVal );

		// save vol & pan for this buffer to be used as next ref level
		g_VolRight = usRightVal;
		g_VolLeft = usLeftVal;
		DEBUGMSG(ZONE_INFO,(TEXT("DsDriverBuffer::SetVolumePan - usRightVal %d, usLeftVal %d \r\n"), usRightVal, usLeftVal));
		break;

	case 1:
		//For the hardware secondary buffer ?
		// No information available on the format of the SRC volume registers..
		// so take a wild guess

		// We'll scale the range (0 to g_VolRight) to the range (0 to 0x1000)
		
		m_pCES1371->SRCRegWrite(SRC_DAC1_VOL_L, ((0x1000 * usLeftVal)/g_VolLeft));
		m_pCES1371->SRCRegWrite(SRC_DAC1_VOL_R, ((0x1000 * usRightVal)/g_VolRight));

		DEBUGMSG(ZONE_INFO,(TEXT("DsDriverBuffer::SetVolumePan - usRightVal %d, usLeftVal %d \r\n"), ((0x1000 * usRightVal)/g_VolRight), ((0x1000 * usLeftVal)/g_VolLeft) ));

		break;

	default:
		return DSERR_GENERIC;
	}

	return hr;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::SetFrequency(
    /* [in] */ DWORD dwFrequency)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::SetFrequency\r\n")));
	
	HRESULT hr = DS_OK;

	// Do stuff to set frequency here.
	if (dwFrequency == m_pwfxFormat->nSamplesPerSec) {
		// for now, all we support is the default format
		// it should be fairly easy to program the 1371's Sample Rate Converter to handle this
		return DS_OK;
	}


	return DSERR_UNSUPPORTED;

	//return hr;

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

SetNotificationPositions is a method on the IDsDriverNotify interface.
If your buffer supports this interfaces (in other words, if your hardware
provides support for alerting that play position has passed a certain point)
you need to implement this method.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverBuffer::SetNotificationPositions(
	/* [in] */ DWORD dwCount,
	/* [in] */ LPCDSBPOSITIONNOTIFY pNotify)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::SetNotificationPositions\r\n")));
	
	HRESULT hr = DS_OK;

	if (m_fIsPlaying) {
		// you're not allowed to set notification positions when playing
		return DSERR_INVALIDCALL;
	}


	// Do whatever you need to do to store the array of positions and the 
	// associated event for each position.

	if( (dwCount == 0) != (pNotify == NULL)) {
		// they have to both be zero or both be non-zero
		return DSERR_INVALIDPARAM;
	}
	if (m_pPlayChunks != NULL) {
		delete [] m_pPlayChunks;
		m_pPlayChunks = NULL;
		m_dwNumberOfChunks = 0;
	}
	m_hEventStop = NULL;

	if (dwCount == 0) {
		// caller just wanted to clear existing notifications. We're set.
		return DS_OK;
	}

	m_dwNumberOfChunks = dwCount;
	if (pNotify[dwCount-1].dwOffset == DSBPN_OFFSETSTOP) {
		// copy out the last entry as a special case for Stop()
		m_hEventStop = pNotify[dwCount-1].hEventNotify;
		m_dwNumberOfChunks --;
	}
	// might just want STOP notifications, in which case, we're done
	if (m_dwNumberOfChunks == 0) {
		return DS_OK;
	}

	m_pPlayChunks = new PLAYCHUNK [m_dwNumberOfChunks];
	if (m_pPlayChunks == 0) {
		m_dwNumberOfChunks = 0;
		return DSERR_OUTOFMEMORY;
	}

	DWORD dwLastOffset = 0;
	DWORD dwSampleSize = m_pwfxFormat->nBlockAlign;
	for (DWORD i = 0; i < m_dwNumberOfChunks; i++) {
		m_pPlayChunks[i].hEventNotify = pNotify[i].hEventNotify;
		// convert client's byte offset into a more useful sample offset
		// round up to next sample
		DWORD dwSampleOffset = (pNotify[i].dwOffset + dwSampleSize - 1) / dwSampleSize;
		m_pPlayChunks[i].dwSampleCount = dwSampleOffset - dwLastOffset - 1; // need N-1 for writing to ES1371
		dwLastOffset = dwSampleOffset;
	}

	return hr;
}


BOOL DsDriverBuffer::Allocate(DWORD dwBytesToAllocate)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverBuffer::Allocate\r\n")));

	ASSERT( ! m_pBits );
	ASSERT( ! m_dwSize );

	m_dwSize = dwBytesToAllocate;

	// for ES1371, the Hardware Secondary Buffer is limited to 64K
	// because the DMA channel can only address so much
	if(dwBytesToAllocate > 0x10000)
		return FALSE;

	//	Allocate memory
	switch(m_channelindex)
	{
	case 0:
		if(g_pDAC0_VirtAddr && dwBytesToAllocate <= PRE_ALLOC_BUFFER_SIZE_DAC0)	// If pre-allocated
		{
			m_PhysAddr = g_pDAC0_PhysAddr;
			m_pBits = (PUCHAR) g_pDAC0_VirtAddr;
			m_bUsingPreAllocMemory = TRUE;
		}
		else
		{
			if(!ReservePhysicalMem(m_dwSize, &m_PhysAddr, (ULONG*) &m_pBits))
				return FALSE;
		}

		//After we allocate memory, we need to associate the buffer with an available dma channel
		m_pCES1371->InitDMAChannel(ES1371_DAC0, 
							  m_PhysAddr, 
							  m_dwSize);
		break;

	case 1:
		if(g_pDAC1_VirtAddr && dwBytesToAllocate <= PRE_ALLOC_BUFFER_SIZE_DAC1)	// If pre-allocated
		{
			m_PhysAddr = g_pDAC1_PhysAddr;
			m_pBits = (PUCHAR) g_pDAC1_VirtAddr;
			m_bUsingPreAllocMemory = TRUE;
		}
		else
		{
			if(!ReservePhysicalMem(m_dwSize, &m_PhysAddr, (ULONG*) &m_pBits))
				return FALSE;
		}

		//After we allocate memory, we need to associate the buffer with an available dma channel
		m_pCES1371->InitDMAChannel(ES1371_DAC1, 
							  m_PhysAddr, 
							  m_dwSize);

		break;

	default:
		return FALSE;
	}

	DEBUGMSG(ZONE_INFO, (TEXT("IDSES1371: buf %d: Buffer size %x and locked at phys addr %x \r\n"),m_channelindex, m_dwSize, m_PhysAddr));
	return TRUE;
}


VOID DsDriverBuffer::ProcessInterrupt()
{
	DWORD dwPlayPosition=0; 
	DWORD i=0;

	// to support variable-sized play-chunks (i.e. notification positions) with perfect precision, do this:
	// - on Play(). load the sample counter with the size of the first chunk
	// - start the DMA channel
	// - wait to ensure that the device has loaded the sample counter into the countdown register
	// - load the sample counter with the size of the *second* chunk
	// - after that, each time you get an interrupt, load the sample counter with the size of the *next* chunk
	//
	// the point is this: when you start playback, the device loads the sample counter into
	// the countdown register (CURR_SAMP_CT in the docs) and uses the countdown register to decide
	// when to fire the interrupt. Once the countdown register is loaded, the sample counter is ignored.
	// When the countdown register reaches zero, the device simultaneously (as far as the SW is concerned)
	// raises the interrupt line, loads the countdown register from the sample counter and starts
	// the countdown to the NEXT interrupt.
	// Note that only later will the interrupt service thread find out about this, which means that
	// by the time the IST runs, the next chunk is already playing, using whatever sample counter value
	// was programmed before the interrupt fired.
	// Writing the sample counter in the IST will **NOT** affect the size of the currently playing chunk,
	// only the size of the subsequent chunk.

	if (m_dwNumberOfChunks > 0)
	{
		DWORD dwNextChunk = m_dwCurrentChunk + 1;
		if (dwNextChunk >= m_dwNumberOfChunks) {
			dwNextChunk -= m_dwNumberOfChunks;
		}
		switch(m_channelindex)
		{
		case 0:
			m_pCES1371->HwRegRMW(ES1371_wDAC0IC_OFF, (USHORT)0xffff, (USHORT)(m_pPlayChunks[dwNextChunk].dwSampleCount));
			//dwPlayPosition = m_pCES1371->GetDMAPosition(ES1371_DAC0) ;
			break;
		case 1:
			m_pCES1371->HwRegRMW(ES1371_wDAC1IC_OFF, (USHORT)0xffff, (USHORT)(m_pPlayChunks[dwNextChunk].dwSampleCount));
			//dwPlayPosition = m_pCES1371->GetDMAPosition(ES1371_DAC1) ;
			break;
		default:
			return;
		}
		// BUGBUG - add diagnostic code to confirm that the play cursor truly is in the expected chunk
		// the events might belong to another process, so set permissions long enough
		// to set the event.
		DWORD dwSavedPermissions = GetCurrentPermissions();
		SetProcPermissions( DWORD (-1) );
		//set off notification events
		SetEvent(m_pPlayChunks[m_dwCurrentChunk].hEventNotify);
		//m_dwOldPlayPos = dwPlayPosition;
		SetProcPermissions( dwSavedPermissions );
		m_dwCurrentChunk = dwNextChunk;
	}

	// now test for looping
	// note that single-shot mode and notification positions are not mutually exclusive
	//If LOOP is not specified (one shot buffers)
	// we'll stop the buffer on an interrupt
	if(!(m_dsbPlayFlags & DSBPLAY_LOOPING))
	{
		//reset the hardware dma channel
		Stop();
	}

}


/*	{	//dump register values
		DWORD dwRegs,dwRegs2;
		dwRegs = READ_PORT_ULONG ( (PULONG)(m_pCES1371->m_pPciAddr + ES1371_bSERFMT_OFF) );
		dwRegs2 = READ_PORT_ULONG ( (PULONG)(m_pCES1371->m_pPciAddr + ES1371_bDEVCTL_OFF) );
		DEBUGMSG(1, (TEXT("DsDriverBuffer::GetPosition--buf %d--size %x, write %x, play %x...ES1371_bSERFMT_OFF: %x - ES1371_bDEVCTL_OFF: %x\r\n"), m_channelindex, m_dwSize, *pdwWrite, *pdwPlay, dwRegs, dwRegs2));
	}
*/
