//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************
/*++

	Module Name:
	    fakehw.h

	Abstract:

		Declarations for various hardware level functions and structures of our fake device

	Environment:

		Win CE 2.11
--*/

#ifndef _FAKEHW_H_
#define _FAKEHW_H_


// general hardware functions
DWORD WriteAC97(PDRIVER_CONTEXT pDriverContext, DWORD ulOffset, DWORD ulValue);
DWORD ReadAC97(PDRIVER_CONTEXT pDriverContext, DWORD ulOffset, PDWORD pdwValue);
DWORD PeekBA0(PDRIVER_CONTEXT pDriverContext, ULONG ulByteOffset, DWORD *pulValue);
DWORD PokeBA0(PDRIVER_CONTEXT pDriverContext, ULONG ulByteOffset, ULONG ulValue);
void PokeBA1 ( PDRIVER_CONTEXT pDriverContext, ULONG ulByteOffset, ULONG ulValue );
void PokeIO(PDRIVER_CONTEXT pDriverContext, ULONG ulAddress, ULONG ulValue);
VOID    AudioPowerOn();

// Hrdw specific hardware functions

DWORD DriverInitialize();
DWORD HrdwInitialize(PDRIVER_CONTEXT pDriverContext);
DWORD HrdwClearSerialFIFOs(PDRIVER_CONTEXT pDriverContext);
DWORD HrdwDownloadImage(PDRIVER_CONTEXT);
void HrdwSetVolume(PDRIVER_CONTEXT pDriverContext, DWORD dwAddress, DWORD dwVolume);
void SetPlaySampleRate(PDRIVER_CONTEXT pDriverContext, DWORD dwBaseRegister, DWORD dwPlayRate);


#endif //_FAKEHW_H_

