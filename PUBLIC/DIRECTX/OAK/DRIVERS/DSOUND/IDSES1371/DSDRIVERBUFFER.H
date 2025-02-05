//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-1999 Microsoft Corporation

Module Name:

	DsDriverBuffer.h

Abstract:

	Declaration file for the 
		
		  DsDriverBuffer 
		  
	class.


Revision History:

	mm 08/19/99

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
#ifndef __DS_DRIVER_BUFFER_H__
#define __DS_DRIVER_BUFFER_H__


//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
//
// where we can find declarations needed in this file:
//
//#include "includes.h"
//#include "Debug.h"
//#include <DSound.h>
//#include <DsDriver.h>
//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////

typedef struct
{
	DWORD	dwSampleCount;	// number of sample in the chunk
	HANDLE	hEventNotify;	// notification event for this chunk
} PLAYCHUNK;

// Forward declaration so the include order works.
class DsDriver;


//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

This class corresponds to your basic hardware sound buffer.  You will have
one instance of this class available for each hardware buffer your device
supports.  Your device may support only one instance of this object.

Currently, this object implements the IDsDriverBuffer and IDsDriverNotify
interfaces.  In the future, you may wish to augment it to also support support
the IDsDriverPropertySet interface to enable 3D and other features.

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
class DsDriverBuffer : public IDsDriverBuffer, public IDsDriverNotify
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
	IDsDriverBuffer methods
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
    STDMETHOD(Lock)                 (THIS_ LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD, DWORD, DWORD );
    STDMETHOD(Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD );
    STDMETHOD(SetFormat)            (THIS_ LPWAVEFORMATEX );
    STDMETHOD(SetFrequency)         (THIS_ DWORD );
    STDMETHOD(SetVolumePan)         (THIS_ PDSVOLUMEPAN );
    STDMETHOD(SetPosition)          (THIS_ DWORD );
    STDMETHOD(GetPosition)          (THIS_ LPDWORD, LPDWORD );
    STDMETHOD(Play)                 (THIS_ DWORD, DWORD, DWORD );
    STDMETHOD(Stop)                 (THIS );
    STDMETHOD(GetStatus)            (THIS_ LPDWORD);



	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
	IDsDriverNotify methods
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
	STDMETHOD(SetNotificationPositions) (THIS_ DWORD, LPCDSBPOSITIONNOTIFY);



	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
	methods public but not exposed through any interfaces
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
	BOOL Allocate(DWORD dwBytesToAllocate);

	VOID ProcessInterrupt();

	//////////////////////////////////////////////////////////////////////////////
	/*****************************************************************************
	Constructor, destructor
	*****************************************************************************/
	//////////////////////////////////////////////////////////////////////////////
	DsDriverBuffer
	(
		HRESULT &hr
	);
	~DsDriverBuffer();



	// Put any public data you need for a buffer here where the DsDriver class can 
	// see it.

	// Pointer to the DsDriver object that created this buffer
	DsDriver *m_pDsDriver;

	// Pointer to the ces1371 hw-driver class
	CES1371* m_pCES1371;

	// Buffer descriptors

	// Pointer to buffer memory and size
	LPBYTE	m_pBits;
	DWORD	m_dwSize;

	// physical address of the (locked)buffer 
	ULONG	m_PhysAddr;	

	// The dma channel the buffer is associated with
	USHORT	m_channelindex;

	// Format of the buffer
	LPWAVEFORMATEX m_pwfxFormat;
	DWORD	m_numberofsamples;

	// Current Next valid Write position, determined after UnLock()
	DWORD	m_dwWritePosition;

	DWORD	m_dwOldPlayPos;

	// Size of data written on the last call to Lock and UnLock
	//	DWORD	m_dwCurrentChunkSize;

	// Current Play flags
	DWORD	m_dsbPlayFlags;

	// True when playing
	BOOL	m_fIsPlaying;

	// TRUE if this buffer is the Primary buffer
	BOOL	m_bPrimary;

	// Current Notification Positions data
	HANDLE	m_hEventStop;
	DWORD	m_dwNumberOfChunks;
	DWORD	m_dwCurrentChunk; // which chunk is currently playing
	PLAYCHUNK * m_pPlayChunks;

	// The Volume scale, linear to log
	DSVOLUMEPAN	m_VolumePan;

	// This variable is used to maintain a reference count for m_pBits in case 
	// DuplicateSoundBuffer is called. Note that since Ensoniq 1371 has only two
	// hardware buffers, only one pair of duplicated buffers can exist and so a 
	// single refcount suffices.
	static DWORD dwCloneRefCount;


	// Use these to maintain a doubly-linked list of all buffers.
	DsDriverBuffer * m_pNext;
	DsDriverBuffer * m_pPrevious;


private:

	ULONG	m_ulRefCount;

	BOOL	m_bUsingPreAllocMemory;

	// Put any private data you need for a buffer here.
	CRITICAL_SECTION m_csDriverBuffer;

};

#endif // __DS_DRIVER_BUFFER_H__

