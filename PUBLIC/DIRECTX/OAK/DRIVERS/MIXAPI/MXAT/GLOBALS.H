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
	    globals.h

	Abstract:

		Declares info and functions for the whole DLL (used by both the mixer and the wave drivers)

	Environment:

		Win CE 2.11
--*/

#ifndef __GLOBALS_H__
#define __GLOBALS_H__


int	  GetCurrentPlatform();



// driver identification

#define DRIVER_VERSION	0x0001
#define DRIVER_WMID MM_CRYSTAL
#define WAVEDRIVER_NAME	TEXT("Sample Audio Driver")

typedef struct tagHWREG
{
	ULONG	offset;
	ULONG	value;
} HWREG;

class HwRegAperture
{
public:
	HwRegAperture(int count);
	~HwRegAperture();
	void Peek(ULONG ulByteOffset, DWORD *pulValue);
	void Poke(ULONG ulByteOffset, DWORD pulValue);

private:
	HWREG * m_reg_table;
	int		m_reg_table_size;
	int		m_reg_count;
};

typedef struct tagMixer_Context *PMIXER_CONTEXT;

typedef struct tagDriverContext
{
	HwRegAperture * Aperture0;
	HwRegAperture * Aperture1;
	HwRegAperture * AC97Regs;

	BOOL	bMixerInitialized;

	BOOL  bZVMute;					// used to save the status of the ZV port

	CRITICAL_SECTION mixCritSec;
	PMIXER_CONTEXT pMixerContextList;
} DRIVER_CONTEXT, *PDRIVER_CONTEXT;

extern PDRIVER_CONTEXT g_pDriverContext;

__inline void LockMixerList() 
{
	if (g_pDriverContext)
		EnterCriticalSection(&g_pDriverContext->mixCritSec);
}

__inline void UnlockMixerList() 
{
	if (g_pDriverContext)
		LeaveCriticalSection(&g_pDriverContext->mixCritSec);
}


DWORD WriteAC97(PDRIVER_CONTEXT pDriverContext, DWORD ulOffset, DWORD ulValue);


#endif //__GLOBALS_H__

