//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************
/*
	Module Name:
		mixdrv.h
	
	Abstract:
		Declarations for the audio mixer drv 
	
*/


#ifndef __MIXDRV_H__
#define __MIXDRV_H__

#include "wavemdd.h"
//#include <mixerdef.h>
// driver functions
extern "C" 
{

DWORD MIX_Init( DWORD regKeyPath );
BOOL MIX_Deinit( DWORD dwContext );
void MIX_PowerDown( DWORD devctx );
void MIX_PowerUp( DWORD devctx );
DWORD MIX_Open( DWORD devctx, DWORD access, DWORD share );
BOOL MIX_Close( DWORD openctx );
DWORD MIX_Read( DWORD openctx, void *pBuf, DWORD len );
DWORD MIX_Seek( DWORD openctx, long int pos, DWORD type );
DWORD MIX_Write( DWORD openctx, void *pBuf, DWORD len );
BOOL MIX_IOControl(
	DWORD openctx,
	DWORD dwCode,
	BYTE  *pBufIn,
	DWORD dwLenIn,
	BYTE  *pBufOut,
	DWORD dwLenOut,
	DWORD *pdwActualOut );

typedef struct tagMixer_Context
{
	HANDLE hProcess;
//	PDRIVER_CONTEXT pDriverContext;	// pointer to the unique driver context structure
	PDRVCALLBACK pfnMixerCallback; 	// sent us from the middle layer
	DWORD dwInstance;	// returned to the application calling us
	struct tagMixer_Context*	pNext;
} MIXER_CONTEXT, *PMIXER_CONTEXT;


}

#endif // __MIXDRV_H__


