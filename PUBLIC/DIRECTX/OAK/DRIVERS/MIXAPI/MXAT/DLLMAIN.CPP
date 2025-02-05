//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************
//
//  Module Name:  
//  
//      mxat.c
//  
//  Abstract:  
//  
//  Functions:
//  
//  Notes:
//  
// -----------------------------------------------------------------------------
#include "wavemdd.h"


BOOL fPowerUp;
// Debug Zones.
//
#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("mxat"), 
	{
		TEXT("Test"),
		TEXT("Params"),
		TEXT("Verbose"),
		TEXT("Interrupt"),
		TEXT("WODM"),
		TEXT("WIDM"),
		TEXT("PDD"),
		TEXT("MDD"),
		TEXT("8"),
		TEXT("Misc"),
		TEXT("10"),
		TEXT("IOcontrol"),
		TEXT("Alloc"),
		TEXT("Function"),
		TEXT("Warning"),
		TEXT("Error") 
	},
	0xc000 // errors and warning on by default
}; 
#endif

// Global

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL WINAPI DllMain (
    HANDLE  hinstDLL,
    DWORD   Op,
    LPVOID  lpvReserved
    )
{
    switch (Op) {

        case DLL_PROCESS_ATTACH :
            DEBUGREGISTER((HINSTANCE)hinstDLL);
            break;

        case DLL_PROCESS_DETACH :
            break;
            
        case DLL_THREAD_DETACH :
            break;
            
        case DLL_THREAD_ATTACH :
            break;
            
        default :
            break;
    }
    return TRUE;
}



