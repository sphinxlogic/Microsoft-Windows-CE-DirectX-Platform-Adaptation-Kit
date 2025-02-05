// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  Copyright (c) 1997-1998  Microsoft Corporation
//
// -----------------------------------------------------------------------------

#ifndef __GPEDDHALUSER_H__
#define __GPEDDHALUSER_H__

// some macros to help in debugging

#define DEBUGENTER(func)                                        \
    {                                                           \
    DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Entering function %s\r\n"),TEXT(#func)));\
    }

#define DEBUGLEAVE(func)                                        \
    {                                                           \
    DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Leaving function %s\r\n"),TEXT( #func )));\
    }


// HARDWARE-SPECIFIC Required #defines
#define IN_VBLANK   \
    (reg_STATUS_1 & 0x08)

#undef IN_DISPLAY
#define IN_DISPLAY   \
    !(reg_STATUS_1 & 0x01)

#define S3D_IS_BUSY     \
    !(reg_MM8504RO & 0x00002000L)

#define IS_BUSY S3D_IS_BUSY

#define DDBLT_FIFO_COUNT    13

#define S3D_FIFO_HAS_ROOM(slots) \
    (((reg_MM8504RO >> 8) & 0x1F) < slots)

#endif //__GPEDDHALUSER_H__
