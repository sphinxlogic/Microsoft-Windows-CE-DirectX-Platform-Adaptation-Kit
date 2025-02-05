//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-1999 Microsoft Corporation

Module Name:

	DsDriverCaptureImp.h

Abstract:

	Declaration file for the 

			DsDriverCapture

	class.


Revision History:

	mm 08/19/99 

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
#ifndef __DS_DRIVERCAPTURE_IMP_H__
#define __DS_DRIVERCAPTURE_IMP_H__


//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
//
// where we can find declarations needed in this file:
//
//#include "DsDriverCaptureBuffer.h"
//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
class DsDriverCapture : public IDsCaptureDriver
{

public:

	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
	IUnknown implementation
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *);
    STDMETHOD_(ULONG,AddRef)        (THIS);
    STDMETHOD_(ULONG,Release)       (THIS);
    
	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
    IDsDriverCapture methods
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
    STDMETHOD(GetDriverDesc)        (THIS_ PDSDRIVERDESC );
    STDMETHOD(Open)                 (THIS );
    STDMETHOD(Close)                (THIS );
    STDMETHOD(GetCaps)              (THIS_ PDSCDRIVERCAPS );
    STDMETHOD(CreateCaptureBuffer)  (THIS_ LPWAVEFORMATEX, DWORD, DWORD, LPDWORD, LPBYTE *, LPVOID * );


	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
    methods public but not exposed on any interfaces
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
 	//STDMETHOD(RemoveBufferFromList)	(THIS_ DsCaptureDriverBuffer *);



	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
	Constructor, destructor
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
	DsDriverCapture(
    	/* [out] */ HRESULT &hr);

	~DsDriverCapture();


	// Put any public data specific to the device here where the other classes
	// in the driver can access it.

	VOID	ProcessInterrupt();

	CES1371* m_pCES1371;

	int m_nActiveBuffers;	//No. of active buffers. should not exceed 2 for es1371

	// pointer to the single capture buffer (just one playback channel on es1371)
	DsCaptureDriverBuffer *m_pCaptureBuffer;

private:

	ULONG m_ulRefCount;


	// Since these will only be called from within this class, they do
	// not need to be published.
	

	// Put any private data specific to the device here.
	// Hide stuff here that the rest of the driver doesn't need to know about.

	int m_fOpened;	//var. used to ensure device is opened before createsoundbuffer

	// Note that drivers need to be thread safe, so we must protect 
	// critical data so it works under access from multiple threads.

	CRITICAL_SECTION m_csSoundBuffer;

};


#endif // __DS_DRIVERCAPTURE_IMP_H__

