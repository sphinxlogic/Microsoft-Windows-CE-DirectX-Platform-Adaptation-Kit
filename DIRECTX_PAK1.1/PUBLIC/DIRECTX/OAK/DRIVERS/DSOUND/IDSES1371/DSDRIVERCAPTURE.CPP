//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-2000 Microsoft Corporation

Module Name:

	DsDriverCapture.cpp

Abstract:

	Implementation file for the DsDriverCapture class.


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
//#include "DsDriverCaptureImp.h"
extern DWORD	 g_IntrAudio;
extern UCHAR	 g_Irq;
extern ULONG	 g_IoBase;
extern CES1371	*g_pCES1371;

//
//
// where we can find declarations needed in this file:
//

//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////

DsDriverCapture::DsDriverCapture
(
	HRESULT &hr
)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::DsDriverCapture\r\n")));
	
	m_pCaptureBuffer = NULL;
	InitializeCriticalSection( &m_csSoundBuffer );

	hr = DS_OK;
	m_ulRefCount = 1;

	m_fOpened = FALSE;
	m_nActiveBuffers = 0;

	// Do initialization stuff that needs to be done whenever the driver is 
	// Open'ed or Close'd in those methods (below).

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
DsDriverCapture::~DsDriverCapture()
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::~DsDriverCapture\r\n")));

    if (m_pCaptureBuffer) {
        m_pCaptureBuffer->Release();
    }

	DeleteCriticalSection( &m_csSoundBuffer );

	if(m_fOpened)
		Close();

}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverCapture::QueryInterface (
	REFIID pIid,
	LPVOID *ppNew )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::QueryInterface\r\n")));
	

    DEBUGCHK(ppNew != NULL);

#ifdef PARAMETER_VALIDATION
    if( ! ppNew )
    {
	    DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::QueryInterface -- ppNew is NULL\r\n")));
        return DSERR_INVALIDPARAM;
    }
#endif

	if( IsEqualIID(IID_IUnknown, pIid) 
		|| IsEqualIID( IID_IDsCaptureDriver, pIid)
		)
	{
		
		InterlockedIncrement( (LPLONG) &m_ulRefCount );

		
		*ppNew = (IDsCaptureDriver *) this;
		return NO_ERROR;
	}

	*ppNew = NULL;
	return DSERR_NOINTERFACE;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsDriverCapture::AddRef(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriverCapture::AddRef previous=%ld\r\n"), m_ulRefCount));
	
	return InterlockedIncrement( (LPLONG) &m_ulRefCount );
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) DsDriverCapture::Release(void)
{
    DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriverCapture::Release previous=%ld\r\n"), m_ulRefCount));
	
	
	ULONG ulRefs = InterlockedDecrement( (LPLONG) &m_ulRefCount );

	if( ulRefs == 0 )
	{
		DEBUGMSG(ZONE_REFCOUNT,(TEXT("DsDriverCapture::Release final on 0x%08x\n"), this ));
		delete this;
	}
	return ulRefs;
}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverCapture::Open(void)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::Open\r\n")));

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

	m_pCES1371->SetCaptureDriver(this, TRUE);

	m_fOpened = TRUE;

	return DS_OK;
}



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverCapture::Close(void)
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::Close\r\n")));

	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	m_pCES1371->SetCaptureDriver(this, FALSE);
	m_fOpened = FALSE;

	return DS_OK;	
}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverCapture::CreateCaptureBuffer( 
								   LPWAVEFORMATEX lpWaveFormatEx
								 , DWORD dwFlags
								 , DWORD dwCardAddress
								 , LPDWORD pdwcbBufferSize
								 , LPBYTE *ppbBuffer
								 , LPVOID *ppvObj 
								 )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::CreateSoundBuffer\r\n")));
	
	CriticalSectionAcquisition csa(&m_csSoundBuffer);

	HRESULT hr;

	if(m_fOpened == FALSE)
		return DSERR_GENERIC; 


	// ES1371 Has a SINGLE channel for capture.
	if(m_nActiveBuffers >= 1) 
		return DSERR_ALLOCATED;


	if( ! lpWaveFormatEx )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateSoundBuffer -- lpWaveFormatEx is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}
	if( ! ppvObj )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateSoundBuffer -- ppvObj is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}
	if( ! pdwcbBufferSize)
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateSoundBuffer -- pdwcbBufferSize is NULL\r\n")));
		return DSERR_INVALIDPARAM;
	}

	// Create the buffer object.

	DsCaptureDriverBuffer *pDsCaptureDriverBuffer = new DsCaptureDriverBuffer(hr);

	if( ! pDsCaptureDriverBuffer || FAILED(hr) )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateSoundBuffer -- unable to create secondary buffer object\r\n")));

		if( pDsCaptureDriverBuffer )
		{
			delete pDsCaptureDriverBuffer;
		}

		if( SUCCEEDED(hr) )
		{
			// Change the error code to reflect the failure.
			hr = DSERR_OUTOFMEMORY;
		}

		return hr;
	}

	// Initialize the buffer

	pDsCaptureDriverBuffer->m_pDsDriverCapture = this;
	pDsCaptureDriverBuffer->m_pCES1371 = m_pCES1371;

	if(dwCardAddress)
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateSoundBuffer -- WARNING dwCardAddress Not supported\r\n")));

	// Allocate buffers
	if( ! pDsCaptureDriverBuffer->Allocate(*pdwcbBufferSize) )
	{
		DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateSoundBuffer -- unable to allocate memory for buffers \r\n")));
		delete pDsCaptureDriverBuffer;
		*pdwcbBufferSize = 0;
		return DSERR_OUTOFMEMORY;
	}

	// Set format
	pDsCaptureDriverBuffer->SetFormat(lpWaveFormatEx);

	// Now call on the object we've created and retrieve the
	// IDsCaptureDriverBuffer interface.
	hr = pDsCaptureDriverBuffer->QueryInterface( IID_IDsCaptureDriverBuffer, ppvObj );


	// Need to make sure RefCount on the object is correct -- whether we
	// succeeded above or failed, it is 1 higher than it should be.
	// If we  succeeded above, it is now 2, not 1 and if we failed it is 1 not 0.
	// This is because both constructor and QueryInterface increased RefCount.
	pDsCaptureDriverBuffer->Release();


	if( FAILED(hr) || ! *ppvObj )
	{
	    DEBUGMSG(ZONE_ERROR,(TEXT("DsDriverCapture::CreateCaptureBuffer -- QueryInterface failed\r\n")));
		if( *ppvObj )
		{
			((IUnknown *) *ppvObj)->Release();
			*ppvObj = NULL;
		}

		delete pDsCaptureDriverBuffer;
		*pdwcbBufferSize = 0;

		return CLASS_E_CLASSNOTAVAILABLE;
	}

	m_pCaptureBuffer = pDsCaptureDriverBuffer;
	return hr;

}	



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverCapture::GetCaps(PDSCDRIVERCAPS pDSCDriverCaps )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::GetCaps\r\n")));
	
	HRESULT hr = DS_OK;


	// Fill out the DSDRIVERCAPS structure with the appropriate information.

	if(! pDSCDriverCaps)
		return DSERR_GENERIC;	//invalid param

	pDSCDriverCaps->dwFlags = 0;
	pDSCDriverCaps->dwChannels = 2;
	pDSCDriverCaps->dwFormats =    WAVE_FORMAT_1M08 
								| WAVE_FORMAT_1M16 
								| WAVE_FORMAT_1S08 
								| WAVE_FORMAT_1S16 
								| WAVE_FORMAT_2M08 
								| WAVE_FORMAT_2M16 
								| WAVE_FORMAT_2S08 
								| WAVE_FORMAT_2S16 
								| WAVE_FORMAT_4M08 
								| WAVE_FORMAT_4M16 
								| WAVE_FORMAT_4S08 
								| WAVE_FORMAT_4S16;
	return hr;
}	




//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP DsDriverCapture::GetDriverDesc(PDSDRIVERDESC pDSDriverDesc )
{
    DEBUGMSG(ZONE_ENTER,(TEXT("DsDriverCapture::GetDriverDesc\r\n")));
	
	HRESULT hr = DS_OK;


	// Fill out the DSDRIVERDESC structure with the appropriate information.

	if(! pDSDriverDesc)
		return DSERR_GENERIC;	//invalid param

	pDSDriverDesc->dwFlags = DSDDESC_DONTNEEDPRIMARYLOCK | DSDDESC_DONTNEEDSECONDARYLOCK;
//    _tcscpy(pDSDriverDesc->szDesc, TEXT("Windows CE Ensoniq 1371 DirectSound Driver"));
//    _tcscpy(pDSDriverDesc->szDrvname,TEXT("IDSES1371.DLL"));
    strcpy(pDSDriverDesc->szDesc, "Windows CE Ensoniq 1371 DirectSound Capture Driver");
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

VOID DsDriverCapture::ProcessInterrupt()
{
    UCHAR intsrc = m_pCES1371->GetInterruptSource();

	//
	// ACK the ISR.
	//

	//
	// Route interrupt processing to appropriate buffer object
	//
	m_pCES1371->AckDMAInterrupt( ES1371_INT_ADC );

	if(m_pCaptureBuffer)
		m_pCaptureBuffer->ProcessInterrupt();

    return;
}
