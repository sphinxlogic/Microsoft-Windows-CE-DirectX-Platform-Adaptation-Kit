//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-2000 Microsoft Corporation

Module Name:

	DsDriver.cpp

Abstract:

	Implementation file for the DsDriver class.


Revision History:

	a-gm 08/19/99 - Based on the new CE DirectSound driver model


*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//// BEGIN INCLUDES
//
// standard includes:
#include "includes.h"

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

extern CES1371	*g_pCES1371;


//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsDriver::DsDriver(
	HRESULT &hr
)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::DsDriver\r\n")));

	InitializeCriticalSection( &m_csSoundBuffer );


	hr = DS_OK;
	m_ulRefCount = 1;

	m_fOpened = FALSE;
	m_nActiveBuffers = 0;

	m_pCES1371 = NULL;

	m_pDriverBuffers[0] = m_pDriverBuffers[1] = NULL;

	// Do initialization stuff that needs to be done whenever the driver is 
	// Open'ed or Close'd in those methods (below).

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsDriver::~DsDriver()
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::~DsDriver\r\n")));

	DeleteCriticalSection( &m_csSoundBuffer );

	if(m_fOpened)
		Close();

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::QueryInterface (
	REFIID pIid,
	LPVOID *ppNew )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::QueryInterface\r\n")));
	

    DEBUGCHK(ppNew != NULL);

#ifdef PARAMETER_VALIDATION
    if( ! ppNew )
    {
	    DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::QueryInterface -- ppNew is NULL\r\n")));
        return DSERR_INVALIDPARAM;
    }
#endif

	if( IsEqualIID(IID_IUnknown, pIid) 
		|| IsEqualIID( IID_IDsDriver, pIid)
		)
	{
		
		InterlockedIncrement( (LPLONG) &m_ulRefCount );

		
		*ppNew = (IDsDriver*) this;
		return NO_ERROR;
	}

	*ppNew = NULL;
	return DSERR_NOINTERFACE;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsDriver::AddRef(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriver::AddRef previous=%ld\r\n"), m_ulRefCount));
	
	return InterlockedIncrement( (LPLONG) &m_ulRefCount );
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsDriver::Release(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriver::Release previous=%ld\r\n"), m_ulRefCount));
	
	
	ULONG ulRefs = InterlockedDecrement( (LPLONG) &m_ulRefCount );

	if( ulRefs == 0 )
	{
		DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriver::Release final on 0x%08x\n"), this ));
		delete this;
	}
	return ulRefs;
}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::Open(void)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::Open\r\n")));

	//
	// Driver dll has already been loaded and this is the first call to open
	//
	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	//return if already opened
	if(m_fOpened)
		return DS_OK;

	if(g_pCES1371 == NULL)
		return DSERR_INVALIDPARAM;

	m_pCES1371 = g_pCES1371;
	if(! m_pCES1371->AudioInitialize())
		return DSERR_GENERIC;

	m_pCES1371->SetDriver(this, TRUE);

	m_fOpened = TRUE;

	return DS_OK;
}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::Close(void)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::Close\r\n")));

	DEBUGMSG(1,(TEXT("DsDriver::Close -- closing Driver\r\n")));
	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	m_pCES1371->SetDriver(this, FALSE);
	m_fOpened = FALSE;

	return DS_OK;	
}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::CreateSoundBuffer( 
								   LPWAVEFORMATEX lpWaveFormatEx
								 , DWORD dwFlags
								 , DWORD dwCardAddress
								 , LPDWORD pdwcbBufferSize
								 , LPBYTE *ppbBuffer
								 , LPVOID *ppvObj 
								 )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::CreateSoundBuffer\r\n")));
	
	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	HRESULT hr;

	if(m_fOpened == FALSE)
		return DSERR_GENERIC;


	// ES1371 Has two channels for output. So the hw can handle 2 buffers.
	// We'll use one for the Primary buffer and the other for a Hardware Secondary Buffer

	if(m_nActiveBuffers >= 2) 
		return DSERR_ALLOCATED;


	if( ! lpWaveFormatEx )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- lpWaveFormatEx is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}
	if( ! ppvObj )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- ppvObj is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}
	if( lpWaveFormatEx->wFormatTag != WAVE_FORMAT_PCM  )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- non WAVE_FORMAT_PCM format not supported\r\n")));
		return DSERR_BADFORMAT;
	}


	// Create the buffer object.

	DsDriverBuffer *pDsDriverBuffer = new DsDriverBuffer(hr);

	if( ! pDsDriverBuffer || FAILED(hr) )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- unable to create secondary buffer object\r\n")));

		if( pDsDriverBuffer )
		{
			delete pDsDriverBuffer;
		}

		if( SUCCEEDED(hr) )
		{
			// Change the error code to reflect the failure.
			hr = DSERR_OUTOFMEMORY;
		}

		return hr;
	}

	// Initialize the buffer

	pDsDriverBuffer->m_pDsDriver	= this;
	pDsDriverBuffer->m_pCES1371		= m_pCES1371;
	pDsDriverBuffer->m_channelindex	= m_nActiveBuffers;

	if(dwCardAddress)
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- WARNING dwCardAddress Not supported\r\n")));

	// Allocate Memory
	if(dwFlags & DSBCAPS_PRIMARYBUFFER)
	{
		// For the primary buffer, we'll decide the size.
		// We'll first try to allocate as much as PRE_ALLOC_BUFFER_SIZE
		// If we fail, we'll try to alloc and lock 2 pages of memory (minimum required)
		// If we still fail, bad luck !

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		#define PRIMARY_BUFFER_PAGE_COUNT	2
		// ..mixer needs at least 2 pages to sound decent

		if( pDsDriverBuffer->Allocate(PRE_ALLOC_BUFFER_SIZE_DAC0) )
		{
			// The pdwcbBufferSize is a return parameter to 
			// tell DSound how big the primary buffer is.
			*pdwcbBufferSize = PRE_ALLOC_BUFFER_SIZE_DAC0;
		}
		else if( ! pDsDriverBuffer->Allocate(PRIMARY_BUFFER_PAGE_COUNT * si.dwPageSize) ) 
		{
			*pdwcbBufferSize = PRIMARY_BUFFER_PAGE_COUNT * si.dwPageSize;
		}
		else
		{
			DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- unable to allocate memory for secondary buffer object\r\n")));
			delete pDsDriverBuffer;
			*pdwcbBufferSize = 0;
			return DSERR_OUTOFMEMORY;
		}

		pDsDriverBuffer->m_bPrimary = TRUE;
	}
	else
	{
		// For the secondary buffer, we'll try to allocate as much as asked

		if( ! pDsDriverBuffer->Allocate(*pdwcbBufferSize) )
		{
			DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- unable to allocate memory for secondary buffer object\r\n")));
			delete pDsDriverBuffer;
			*pdwcbBufferSize = 0;
			return DSERR_OUTOFMEMORY;
		}
	}

	pDsDriverBuffer->SetFormat(lpWaveFormatEx);

	// Now call on the object we've created and retrieve the
	// IDsDriverBuffer interface.
	hr = pDsDriverBuffer->QueryInterface( IID_IDsDriverBuffer, ppvObj );


	// Need to make sure RefCount on the object is correct -- whether we
	// succeeded above or failed, it is 1 higher than it should be.
	// If we  succeeded above, it is now 2, not 1 and if we failed it is 1 not 0.
	// This is because both constructor and QueryInterface increased RefCount.
	pDsDriverBuffer->Release();

	
	if( FAILED(hr) || ! *ppvObj )
	{
	    DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- QueryInterface failed\r\n")));
		if( *ppvObj )
		{
			((IUnknown *) *ppvObj)->Release();
			*ppvObj = NULL;
		}

		delete pDsDriverBuffer;
		*pdwcbBufferSize = 0;

		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Add buffer to our array of buffers
	SetBuffer(m_nActiveBuffers, pDsDriverBuffer, TRUE);

	return hr;

}	




//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::DuplicateSoundBuffer(
			/* [in] */  PIDSDRIVERBUFFER pIDsDriverBuffer,
			/* [out] */	LPVOID *ppv
			)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::DuplicateSoundBuffer\r\n")));
	
	HRESULT hr = DS_OK;
	DEBUGMSG(1,(TEXT("DsDriver::DuplicateSoundBuffer\r\n")));

	// Do what you need to do here to create a new IDsDriverBuffer object
	// which represents a duplicate of the actual sound data in an 
	// already existing object.

	// You will most likely want to share the actual buffer that contains
	// the audio data between the two buffer.

	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	if(m_fOpened == false)
		return DSERR_GENERIC;


	// ES1371 Has two channels for output. So the hw can handle 2 buffers.
	// We'll use one for the Primary buffer and the other for a Hardware Secondary Buffer

	if(m_nActiveBuffers >= 2) 
		return DSERR_ALLOCATED;


	if( ! pIDsDriverBuffer )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::DuplicateSoundBuffer -- pIDsDriverBuffer is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}
	if( ! ppv )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::DuplicateSoundBuffer -- ppv is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}


	// Create the buffer object.

	DsDriverBuffer *pDsDriverBuffer = new DsDriverBuffer(hr);

	if( ! pDsDriverBuffer || FAILED(hr) )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::DuplicateSoundBuffer -- unable to create secondary buffer object\r\n")));

		if( pDsDriverBuffer )
		{
			delete pDsDriverBuffer;
		}

		if( SUCCEEDED(hr) )
		{
			// Change the error code to reflect the failure.
			hr = DSERR_OUTOFMEMORY;
		}

		return hr;
	}
	// Add the DsDriverBuffer object into the list of buffers managed by
	// this IDsDriver object.  
	// Note: We need to do this before we call InternalInitialize because
	// if any of the methods below fail, the destructor on the
	// DsDriverBuffer  object will attempt to call RemoveBufferFromList.

/*	hr = AddBufferToList( pDsDriverBuffer );

	if( FAILED( hr ) )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::DuplicateSoundBuffer -- error adding buffer to list\r\n")));

		((IUnknown *) *ppv)->Release();

		*ppv = NULL;

		return hr;
	}
*/
	// Initialize the buffer

	{
		// copy all data of pIDsDriverBuffer to this
		// note, only the buffer is shared
		DsDriverBuffer *pDsParentbuffer = (DsDriverBuffer *)pIDsDriverBuffer;
		pDsDriverBuffer->m_pDsDriver	= this;
		pDsDriverBuffer->m_pCES1371		= m_pCES1371;
		pDsDriverBuffer->m_channelindex	= m_nActiveBuffers;
		pDsDriverBuffer->m_pBits		= pDsParentbuffer->m_pBits;
		pDsDriverBuffer->m_dwSize		= pDsParentbuffer->m_dwSize ;
		pDsDriverBuffer->m_PhysAddr		= pDsParentbuffer->m_PhysAddr ;
		pDsDriverBuffer->m_bPrimary		= FALSE;

		pDsDriverBuffer->SetFormat(pDsParentbuffer->m_pwfxFormat);
	}

	// Now call on the object we've created and retrieve the
	// IDsDriverBuffer interface.
	hr = pDsDriverBuffer->QueryInterface( IID_IDsDriverBuffer, ppv );


	// Need to make sure RefCount on the object is correct -- whether we
	// succeeded above or failed, it is 1 higher than it should be.
	// If we  succeeded above, it is now 2, not 1 and if we failed it is 1 not 0.
	// This is because both constructor and QueryInterface increased RefCount.
	pDsDriverBuffer->Release();

	
	if( FAILED(hr) || ! *ppv )
	{
	    DEBUGMSG(ZONE_ERROR,(TEXT("DsDriver::CreateSoundBuffer -- QueryInterface failed\r\n")));
		if( *ppv )
		{
			((IUnknown *) *ppv)->Release();
			*ppv = NULL;
		}

		delete pDsDriverBuffer;

		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Since the buffer m_pBits is shared for duplicated buffers we need to maintain a count
	{
		DsDriverBuffer::dwCloneRefCount ++;
	}

	// Add buffer to our array of buffers
	SetBuffer(m_nActiveBuffers, pDsDriverBuffer, TRUE);

	return hr;

}	





//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::GetCaps(PDSDRIVERCAPS pDSDriverCaps )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::GetCaps\r\n")));
	
	HRESULT hr = DS_OK;


	// Fill out the DSDRIVERCAPS structure with the appropriate information.

	if(! pDSDriverCaps)
		return DSERR_GENERIC;	//invalid param

	pDSDriverCaps->dwFlags =  DSCAPS_CONTINUOUSRATE 
							| DSCAPS_PRIMARYMONO   | DSCAPS_PRIMARYSTEREO   
							| DSCAPS_PRIMARY8BIT   | DSCAPS_PRIMARY16BIT   
							| DSCAPS_SECONDARYMONO | DSCAPS_SECONDARYSTEREO 
							| DSCAPS_SECONDARY8BIT | DSCAPS_SECONDARY16BIT
							;

	pDSDriverCaps->dwMinSecondarySampleRate = 1;
	pDSDriverCaps->dwMaxSecondarySampleRate = 48000;
	pDSDriverCaps->dwPrimaryBuffers = 1;
	pDSDriverCaps->dwMaxHwMixingAllBuffers = 2;
	pDSDriverCaps->dwMaxHwMixingStaticBuffers = 0;
	pDSDriverCaps->dwMaxHwMixingStreamingBuffers = 2;
	pDSDriverCaps->dwFreeHwMixingAllBuffers = 2 - m_nActiveBuffers;
	pDSDriverCaps->dwFreeHwMixingStaticBuffers = 0;
	pDSDriverCaps->dwFreeHwMixingStreamingBuffers = 2 - m_nActiveBuffers;
	pDSDriverCaps->dwMaxHw3DAllBuffers = 0;
	pDSDriverCaps->dwMaxHw3DStaticBuffers = 0;
	pDSDriverCaps->dwMaxHw3DStreamingBuffers = 0;
	pDSDriverCaps->dwFreeHw3DAllBuffers = 0;
	pDSDriverCaps->dwFreeHw3DStaticBuffers = 0;
	pDSDriverCaps->dwFreeHw3DStreamingBuffers = 0;
	pDSDriverCaps->dwTotalHwMemBytes = 0;
	pDSDriverCaps->dwFreeHwMemBytes = 0;
	pDSDriverCaps->dwMaxContigFreeHwMemBytes = 0;

	return hr;
}	




//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriver::GetDriverDesc(PDSDRIVERDESC pDSDriverDesc )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriver::GetDriverDesc\r\n")));
	
	HRESULT hr = DS_OK;


	// Fill out the DSDRIVERDESC structure with the appropriate information.

	if(! pDSDriverDesc)
		return DSERR_GENERIC;	//invalid param

	pDSDriverDesc->dwFlags = DSDDESC_DONTNEEDPRIMARYLOCK | DSDDESC_DONTNEEDSECONDARYLOCK;
//    _tcscpy(pDSDriverDesc->szDesc, TEXT("Windows CE Ensoniq 1371 DirectSound Driver"));
//    _tcscpy(pDSDriverDesc->szDrvname,TEXT("IDSES1371.DLL"));
    strcpy(pDSDriverDesc->szDesc, "Windows CE Ensoniq 1371 DirectSound Driver");
//    strcpy(pDSDriverDesc->szDrvname,"IDSES1371.DLL");
	strcpy(pDSDriverDesc->szDrvname,"IDSES1371.DLL");
    pDSDriverDesc->dnDevNode = 0;
    pDSDriverDesc->wVxdId = 0;
    pDSDriverDesc->wReserved = 0;
    pDSDriverDesc->ulDeviceNum = 0;
    pDSDriverDesc->dwHeapType = DSDHEAP_NOHEAP;
    pDSDriverDesc->pvDirectDrawHeap = 0;
    pDSDriverDesc->dwMemStartAddress = 0;
    pDSDriverDesc->dwMemEndAddress = 0;
    pDSDriverDesc->dwMemAllocExtra = 0;
    pDSDriverDesc->pvReserved1 = 0;
    pDSDriverDesc->pvReserved2 = 0;

	return hr;
}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
VOID DsDriver::SetBuffer(USHORT index, DsDriverBuffer *pDsDriverBuffer, BOOL bSet )
{
	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	if(bSet)
	{
		m_pDriverBuffers[index] = pDsDriverBuffer;
		m_nActiveBuffers++;
	}
	else
	{
		m_pDriverBuffers[index] = NULL;
		m_nActiveBuffers--;
	}
}


VOID DsDriver::ProcessInterrupt(UCHAR intsrc)
{
	CriticalSectionAcquisition csa(&m_csSoundBuffer);

    if ( intsrc & ES1371_INT_DAC0 )
    { 
        m_pCES1371->AckDMAInterrupt( ES1371_INT_DAC0);
		if(m_pDriverBuffers[0])
			m_pDriverBuffers[0]->ProcessInterrupt();
    }
    if ( intsrc & ES1371_INT_DAC1 )
    { 
        m_pCES1371->AckDMAInterrupt( ES1371_INT_DAC1);
		if(m_pDriverBuffers[1])
			m_pDriverBuffers[1]->ProcessInterrupt();
    }

    return;
}

