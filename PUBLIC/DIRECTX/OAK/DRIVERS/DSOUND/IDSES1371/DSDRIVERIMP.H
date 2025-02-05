//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-1999 Microsoft Corporation

Module Name:

	DsDriverImp.h

Abstract:

	Declaration file for the 

			DsDriver

	class.


Revision History:

	mm 08/19/99 

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
#ifndef __DS_DRIVER_IMP_H__
#define __DS_DRIVER_IMP_H__


//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
//
// where we can find declarations needed in this file:
//
//#include "DsDriverBuffer.h"
//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
class DsDriver : public IDsDriver
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
    IDsDriver methods
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
    STDMETHOD(GetDriverDesc)        (THIS_ PDSDRIVERDESC );
    STDMETHOD(Open)                 (THIS );
    STDMETHOD(Close)                (THIS );
    STDMETHOD(GetCaps)              (THIS_ PDSDRIVERCAPS );
    STDMETHOD(CreateSoundBuffer)    (THIS_ LPWAVEFORMATEX, DWORD, DWORD, LPDWORD, LPBYTE *, LPVOID * );
    STDMETHOD(DuplicateSoundBuffer) (THIS_ PIDSDRIVERBUFFER, LPVOID * );


	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
    methods public but not exposed on any interfaces
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
	Constructor, destructor
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
	DsDriver(
    	/* [out] */ HRESULT &hr);

	~DsDriver();


	// Put any public data specific to the device here where the other classes
	// in the driver can access it.

	VOID ProcessInterrupt(UCHAR intsrc);

	CES1371	*m_pCES1371;

	int m_nActiveBuffers;	//No. of active buffers. should not exceed 2 for es1371
	VOID SetBuffer(USHORT index, DsDriverBuffer *pDsDriverBuffer, BOOL bSet );


private:

	ULONG m_ulRefCount;

	// We'll have two driverbuffers only, for the two output dma channels
	DsDriverBuffer *m_pDriverBuffers[2];

	// Put any private data specific to the device here.
	// Hide stuff here that the rest of the driver doesn't need to know about.

	int m_fOpened;	//var. used to ensure device is opened before createsoundbuffer


	// Note that drivers need to be thread safe, so we must protect 
	// critical data so it works under access from multiple threads.
	CRITICAL_SECTION m_csSoundBuffer;
};


#endif // __DS_DRIVER_IMP_H__

