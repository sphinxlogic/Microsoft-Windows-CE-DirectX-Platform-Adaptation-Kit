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
//      IrqMap.cpp
//
//  Abstract:
//      This library implements the MapInterruptToSystem method, which
//      is fundamentally a platform-specific function, in a quasi-platform-independent manner.
//      The code here works for CEPC and certain Microsoft internal reference
//      platforms.  For other platforms, this function should either
//      duplicate or call into the platform-specific code that maps IRQ numbers to
//      interrupt IDs that can be passed to InterruptInitialize()
//
//  Functions:
//
//  Notes:
//
// -----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>		// For SYSINTR_FIRMWARE





extern "C" BOOL
MapInterruptToSystem
	(
    IN ULONG InterruptNumber,
    IN ULONG InterruptVector,
    OUT DWORD *InterruptId
    )
{

#if defined(_M_IX86)
	// Here we make a safe but technically invalid assumption:
	// the target CPU is x86, ergo it must be a CEPC platform.
	// On CEPC, mapping IRQ to Interrupt IDs is easy: just
	// and SYSINTR_FIRMWARE to the interrupt number.
	// For other platforms, the logic could be more complex.

    if( InterruptNumber<=15 )
    {
        *InterruptId =  SYSINTR_FIRMWARE + InterruptNumber;
    }
    else
    {
        *InterruptId = 0xffffffff;
    }
    return TRUE;

#else
	// If it's not an x86, let's just make a wild guess that
	// we're working with one of the Microsoft internal
	// reference platforms and that we have a beast called
    // wdmlib that exposes the true platform-specific HalMapInterrupt.
	// Forward the call there...

typedef BOOL (*PFNHalMapInterrupt)(ULONG, ULONG, DWORD *);

    HINSTANCE           hwdmlib;
    PFNHalMapInterrupt  pHalMapInterrupt;
    BOOL                bRet = FALSE;
    BOOL                bOK = FALSE;

    hwdmlib = LoadLibrary(L"wdmlib.dll");
    if (hwdmlib)
    {
        // get all the proc addresses
        pHalMapInterrupt = (PFNHalMapInterrupt)GetProcAddress(hwdmlib, TEXT("HalMapInterrupt"));
        if (pHalMapInterrupt)
        {
            bRet = (*pHalMapInterrupt)(InterruptNumber, InterruptVector, InterruptId);
            bOK = TRUE;
        }

        // free up the library
        FreeLibrary(hwdmlib);
        hwdmlib = NULL;
    }

    if (bOK)
    {
        return bRet;
    }
    else
    {
        if (InterruptVector != 0)
        {
            return FALSE;
        }

        // try loading and using Platform pwdmlib.dll

        if (InterruptId == NULL)
        {
            // Unmapping the previous interrupt
        }
        else
        {
            *InterruptId = SYSINTR_FIRMWARE + InterruptNumber;
        }
    }
    return TRUE;

#endif // CEPC


}
