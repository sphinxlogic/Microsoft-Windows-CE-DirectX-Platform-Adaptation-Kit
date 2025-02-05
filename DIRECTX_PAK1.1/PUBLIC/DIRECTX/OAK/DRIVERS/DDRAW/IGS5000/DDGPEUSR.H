// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  Copyright (c) 1997-2000  Microsoft Corporation
//
// -----------------------------------------------------------------------------

#ifndef __GPEDDHALUSER_H__
#define __GPEDDHALUSER_H__

// some macros to help in debugging

#define DEBUGENTER(func) 										\
	{															\
		DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Entering function %s\r\n"),TEXT(#func)));		\
	}

#define DEBUGLEAVE(func) 										\
	{															\
		DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Leaving function %s\r\n"),TEXT( #func )));	\
	}


// HARDWARE-SPECIFIC Required #defines
#define IN_VBLANK   \
    ((PORT_RO_8 ((GPEVGA *)g_pGPE)->m_VGAIOPort[PortId_3DA]) & 0x08)
#define IS_BUSY     \
    ((MM_RW_8 ((IGS5000 *)g_pGPE)->m_pMMIO[0x0011]) & 0x80)



#endif //__GPEDDHALUSER_H__
