/////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-2000 Microsoft Corporation

Module Name:

	IDsSampleDLL.cpp

Abstract:

	Provides the DllEntry point for the the IDsSample dll.
	This Dll serves out IDsDriver and IDsDriverBuffer
	COM objects which will talk to your audio hardware.


	Software Layers:

       ===============    ===============
       | Application |    | Application |  Module(s)
       ===============    ===============
      
         ----------- DSOUND.H ---------    Interface
      
       ===============    ===============
       | DSOUNDC.DLL |    | DSOUNDC.DLL |  Module (dsoundc.dll)
       ===============    ===============
      
         ------- DSoundSIOCTLs.H -----    Interface
      
         ------ DeviceIoControl() -----    PSL API
      
       ==================================
       |           DSound server        |  Module (dsounds.dll)
       ==================================

         -- IDsDriver, IDsDriverBuffer -  Interface
      
       ==================================
       |            IDsSample.DLL        |  Module (IDsSample.dll)
       ==================================
    
       ==================================
       |            HARDWARE             |
       ==================================



Revision History:

	mm 08/19/99



*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
//
// where we can find declaration for main class in this file:
//
//
//
// where we can find declarations needed in this file:
//
#define INITGUID	// Instantiate the guids from this file - don't do this in two places
#define INSTANTIATE_DEBUG

#include "includes.h"

//#include "DsDriverImp.h"
//
//
//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
globals
*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
HANDLE g_hInstance;

// Global pointer to the CES1371 object
CES1371	*g_pCES1371;

// Addresses of pre-allocated memory for the 3 channels
ULONG	g_pDAC0_PhysAddr;
ULONG	g_pDAC0_VirtAddr;
ULONG	g_pDAC1_PhysAddr;
ULONG	g_pDAC1_VirtAddr;
ULONG	g_pADC_PhysAddr;
ULONG	g_pADC_VirtAddr;

USHORT	g_VolLeft;
USHORT	g_VolRight;

//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
IDsSample.DLL entry point.
*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
extern "C" BOOL
DllEntry(
    HANDLE  hinstDll,                 /*@parm Instance pointer. */
    DWORD   dwReason,                 /*@parm Reason routine is called. */
    PULONG  lpReserved                /*@parm system parameter. */
    )
{

    if ( dwReason == DLL_PROCESS_ATTACH ) 
	{
		DEBUGREGISTER((HINSTANCE)hinstDll);
	    DEBUGMSG(ZONE_ENTER,(TEXT("IDSES1371 process attach\r\n")));


        g_hInstance = hinstDll;

		//
		// Create an ES1371 object to control the hardware 
		//
		g_pCES1371 = (CES1371 *)new CES1371();
		if(! g_pCES1371)
			return FALSE;

		//
		// We'll try and reserve physical memory since it is more likely to succeed
		// on driver load
		//
		g_pDAC0_PhysAddr= g_pDAC0_VirtAddr = NULL;
		g_pDAC1_PhysAddr= g_pDAC1_VirtAddr = NULL;
		g_pADC_PhysAddr = g_pADC_VirtAddr  = NULL;

		ReservePhysicalMem(PRE_ALLOC_BUFFER_SIZE_DAC0, &g_pDAC0_PhysAddr, &g_pDAC0_VirtAddr);
		ReservePhysicalMem(PRE_ALLOC_BUFFER_SIZE_DAC1, &g_pDAC1_PhysAddr, &g_pDAC1_VirtAddr);
		ReservePhysicalMem(PRE_ALLOC_BUFFER_SIZE_ADC, &g_pADC_PhysAddr, &g_pADC_VirtAddr);

		g_VolLeft = (ES1371_INITIALVOLUME >> 8);
		g_VolRight = (ES1371_INITIALVOLUME & 0x00ff);

    }
    if ( dwReason == DLL_PROCESS_DETACH  ) 
	{
	    DEBUGMSG(ZONE_ENTER,(TEXT("IDSES1371 API process detach\r\n")));

		if(g_pCES1371)
			delete g_pCES1371;
	}

    return TRUE;
}



// This is the CLSID that we will register in the appropriate DirectSound
// registry setting.  DirectSound will look in that (to be determined) regsitry
// key, see the name of the dll we have provided, and then create the IDsDriver
// object by calling DllGetClassObject, passing in the CLSID we've registered.

// {EFE5F0F1-568A-11d3-AC7F-00105A017FE1}
DEFINE_GUID(CLSID_ES1371IDsDriver, 
0xefe5f0f1, 0x568a, 0x11d3, 0xac, 0x7f, 0x00, 0x10, 0x5a, 0x01, 0x7f, 0xe1);

// {C5225EF4-0D6C-4327-82C1-578A70BBF09F}
DEFINE_GUID(CLSID_ES1371IDsCaptureDriver, 
0xC5225EF4, 0x0D6C, 0x4327, 0x82, 0xC1, 0x57, 0x8A, 0x70, 0xBB, 0xF0, 0x9F);


//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
HRESULT WINAPI DllGetClassObject(  
	  REFCLSID rclsid  //CLSID for the class object
	, REFIID riid      //Reference to the identifier of the interface 
					// that communicates with the class object
	, LPVOID * ppv      //Address of output variable that receives the 
                    // interface pointer requested in riid
	)
{                    
    DEBUGMSG(ZONE_ENTER,(TEXT("IDsSample DllGetClassObject\r\n")));

	HRESULT hr;

	IUnknown * pUnknown = NULL;


	//## TO DO : for now we just create a new DsDriver object and return.
	//		CLSIDs to be worked out later

	if(IsEqualGUID( rclsid, CLSID_ES1371IDsDriver ) )
	{
		pUnknown = (IUnknown *) new DsDriver( hr );
	}

	//	If we supported creating any other objects from this driver, 
	//  for example a capture driver object (IDsCaptureDriver) then
	//	we could check for that here.
	else if( IsEqualGUID( rclsid, CLSID_ES1371IDsCaptureDriver ) )
	{
		pUnknown = (IUnknown *) new DsDriverCapture( hr );
	}


	if( FAILED(hr) || ! pUnknown )
	{
	    DEBUGMSG(ZONE_ERROR,(TEXT("IDsSample DllGetClassObject -- unknown class\r\n")));
		if( pUnknown )
		{
			delete pUnknown;
		}
		*ppv = NULL;
		return CLASS_E_CLASSNOTAVAILABLE;
	}


	// Now call on the object we've created and see if it supports the 
	// requested interface (i.e. see if it inherits from the
	// requested abstract C++ base class).
	hr = pUnknown->QueryInterface( riid, ppv );

	
	// Need to make sure RefCount on the object is correct -- whether we
	// succeeded above or failed, it is 1 higher than it should be.
	// If we  succeeded above, it is now 2, not 1 and if we failed it is 1 not 0.
	// This is because both constructor and QueryInterface increased RefCount.
	pUnknown->Release();


	if( FAILED(hr) || ! *ppv )
	{
	    DEBUGMSG(ZONE_ERROR,(TEXT("IDsSample DllGetClassObject -- QueryInterface failed\r\n")));
		if( *ppv )
		{
			((IUnknown *) *ppv)->Release();
			*ppv = NULL;
		}

		// ISSUE: Actually interface not supported.
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	
	// If we got to this point, we are good to go.
	
	return DS_OK;

}


//
// Allocates a chunk of memory, locks it and returns physical & virtual addresses
//
BOOL ReservePhysicalMem(DWORD dwSize, ULONG *ppPhysAddr, ULONG *ppVirtAddr)
{

	UINT cPages;
	PULONG aPages;
    BOOL bRetVal;

	//
	// Allocate an Ouput buffer
	//
	*ppVirtAddr = (ULONG)VirtualAlloc(0,
									  dwSize,
									  MEM_COMMIT ,
									  PAGE_READWRITE | PAGE_NOCACHE);
	if (!*ppVirtAddr)  {
		DEBUGMSG(ZONE_ERROR, (TEXT("IDSES1371:ReservePhysicalMem:Allocate: Buffer Allocation Failed")));
		return FALSE;
	}

    // let's first find the maximum # of pages that could be locked
    cPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(NULL, dwSize);

    // allocate an array for the maximum # of pages
    aPages = new ULONG[cPages];
	if(!aPages)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("IDSES1371:ReservePhysicalMem: Failed to allocate Pageframe array ")));
		VirtualFree((VOID *)(*ppVirtAddr), dwSize, MEM_RELEASE);
		*ppPhysAddr = *ppVirtAddr = NULL;
		return FALSE;
	}


    bRetVal = LockPages((VOID *)(*ppVirtAddr), dwSize, aPages, LOCKFLAG_WRITE);
	if (bRetVal != TRUE )  {
		DEBUGMSG(ZONE_ERROR, (TEXT("IDSES1371:ReservePhysicalMem: LockPages Failed")));
		VirtualFree((VOID *)(*ppVirtAddr), dwSize, MEM_RELEASE);
		delete aPages;
		*ppPhysAddr = *ppVirtAddr = NULL;
		return FALSE;
	}

	// LockPages fills aPages[] with the PFNs (Page Frame Numbers?) of the locked pages.
	// We need to left-shift by a magic number to get the actual physical address.
	// Note that as of this writing, the PB online help incorrectly states that we should right-shift.
	// Note also that this code *assumes* that LockPages will lock down contiguous pages, which is not guaranteed
	// we should test that the pages are in fact contiguous and, if not, unlock them and try again,
	// probably with a single-page buffer.
	// kl  7/26/99

    *ppPhysAddr = aPages[0] << UserKInfo[KINX_PFN_SHIFT];
	DEBUGMSG(ZONE_INFO, (TEXT("IDSES1371:ReservePhysicalMem: Buffer Physical Address  %08x\r\n"), *ppPhysAddr));

	//get page size
	SYSTEM_INFO siSystemInfo;
	GetSystemInfo (&siSystemInfo);

	UINT i=0;
	BOOL bValid = TRUE;
	while(i < cPages-1) 
	{ 
		if((aPages[i] << UserKInfo[KINX_PFN_SHIFT]) + siSystemInfo.dwPageSize != (aPages[i+1] << UserKInfo[KINX_PFN_SHIFT]) )
		{
			//error
			bValid = FALSE;
			break;
		}
		i++; 
	}

	delete aPages;

	if(bValid == FALSE)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("IDSES1371:ReservePhysicalMem: Couldn't lock %d contiguous pages \r\n"), cPages));
		UnlockPages((VOID *)(*ppVirtAddr), dwSize);
		VirtualFree((VOID *)(*ppVirtAddr), dwSize, MEM_RELEASE);
		*ppPhysAddr = *ppVirtAddr = NULL;
		return FALSE;
	}
	else
	{
		DEBUGMSG(ZONE_INFO, (TEXT("IDSES1371:ReservePhysicalMem: locked %d contiguous pages \r\n"), cPages));
	}

	memset((VOID *)(*ppVirtAddr), 0, dwSize);

	return TRUE;
}
