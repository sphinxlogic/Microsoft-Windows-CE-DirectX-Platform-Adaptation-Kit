// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//  
//  Module Name:  
//  
//      IrqMap.lib
//  
//  Abstract:  
//      This static library implements the MapInterruptToSystem method, which
//      can be used on any platform to correctly map IRQ's to system interrupts.
//      It takes the place of MapIrq2SysIntr on CEPC and HalMapInterrupt
//		on other platforms.
//  
//  Notes:
//  
// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
#endif
BOOL
MapInterruptToSystem
	(
    IN ULONG InterruptNumber,
    IN ULONG InterruptVector,
    OUT DWORD *InterruptId
    );
