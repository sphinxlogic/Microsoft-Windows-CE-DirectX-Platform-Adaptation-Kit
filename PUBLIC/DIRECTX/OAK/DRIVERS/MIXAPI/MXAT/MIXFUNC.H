//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Module Name:

	mixfunc.h

Abstract:

	Implements mixer functionality

-------------------------------------------------------------------*/



#ifndef __MIXFUNC_H__
#define __MIXFUNC_H__

#define MAX_SUPPORTED_INSTANCES	1	// currently the mixer driver only supports one instance for each process

// those functions handle different requests transmitted as IOCTL calls

//standard documented calls
DWORD mix_handle_GETNUMDEVS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_GETDEVCAPS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_OPEN(PMIXER_CONTEXT pMixer, DWORD *pdwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_CLOSE(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_GETLINEINFO(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_GETLINECONTROLS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_GETCONTROLDETAILS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD mix_handle_SETCONTROLDETAILS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

// mixer utility functions
void mix_InitMixer( PDRIVER_CONTEXT pDriverContext );
BOOL mix_FreeMixer(PMIXER_CONTEXT pMixer);

#endif //__MIXFUNC_H__

