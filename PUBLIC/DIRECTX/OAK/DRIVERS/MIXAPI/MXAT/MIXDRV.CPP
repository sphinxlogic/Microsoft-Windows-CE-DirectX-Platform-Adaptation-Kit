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

		globals.cpp

	Abstract:

		Implements the WinCE audio mixer device driver entry points

		Both the mixer device and the wave audio device use the same hardware
		Therefore the two logical devices were combined 
		into a single module that exposes both the mixer device interface (proprietary
		at this moment) and the wave interface (fairly well documented and public on MSDN)
		This dual nature causes some problems, because we don't want to modify the 
		MDD part of the wave driver; the PDD and mixer work together, but the MDD 
		is the same as the one in the Microsoft samples.

	Environment:

		Win CE 2.11

--*/

#include <windows.h>
#include <mmsystem.h>
#include "mixdrv.h"
#include "globals.h"
#include "mixfunc.h"
#include <wavedbg.h>

PDRIVER_CONTEXT g_pDriverContext = NULL;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Init

Description:

	

Arguments:

	DWORD regKeyPath :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD MIX_Init( DWORD regKeyPath )
{
	DEBUGMSG( ZONE_FUNCTION, (TEXT("+MIX_Init: '%s'\r\n"), (TCHAR*)regKeyPath) );
	if (!g_pDriverContext)
	{	// we did not yet initialize anything;
		g_pDriverContext = new DRIVER_CONTEXT;
		if (!g_pDriverContext)	{
			ERRORMSG(ZONE_ERROR, (TEXT("Error allocating g_pDriverContext\r\n")));
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		g_pDriverContext->Aperture0 = new HwRegAperture(100);
		g_pDriverContext->Aperture1 = new HwRegAperture(100);
		g_pDriverContext->AC97Regs = new HwRegAperture(100); // pretend AC97 is memory mapped
		g_pDriverContext->bMixerInitialized = FALSE;
		g_pDriverContext->bZVMute = TRUE;
	}

	if (g_pDriverContext->bMixerInitialized)
	{
		// somebody tries to INIT us again! That shouldn't happen, so 
		// we ASSERT (to help debugging), but accept anyway
		ASSERT(FALSE);
		return (DWORD)g_pDriverContext;
	}

	g_pDriverContext->bMixerInitialized = TRUE;
	g_pDriverContext->pMixerContextList = NULL;				// init to zero, gets another value on open

	InitializeCriticalSection( &g_pDriverContext->mixCritSec );

	mix_InitMixer(g_pDriverContext);

	DEBUGMSG(ZONE_FUNCTION, (TEXT("MIX_Init: success 0x%8.8X\r\n"), (DWORD)g_pDriverContext));
	return (DWORD)g_pDriverContext;		// success at this point;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Deinit

Description:

	

Arguments:

	DWORD devctx :

Return Value: BOOL
-------------------------------------------------------------------*/
BOOL MIX_Deinit( DWORD devctx )
{
	BOOL result;

	DEBUGMSG( ZONE_FUNCTION, (TEXT("+MIX_Deinit: 0x%08x\r\n"), devctx) );

	if (devctx == 0)	// initialization failed; get out
		return TRUE;

	if( devctx == (DWORD)g_pDriverContext)
	{
		result = TRUE;							// success = TRUE
		DeleteCriticalSection( &g_pDriverContext->mixCritSec );

		if( g_pDriverContext )
		{
			PMIXER_CONTEXT pNext, pMix = g_pDriverContext->pMixerContextList;
			g_pDriverContext->bMixerInitialized = FALSE;

			ASSERT(NULL == pMix);	// all devices should have been closed before here

			while (pMix)
			{
				pNext = pMix->pNext;
				LocalFree(pMix);
				pMix = pNext;
			}
			LocalFree(g_pDriverContext);
			g_pDriverContext = NULL;
		}
	}
	else
	{
		result = FALSE;
		SetLastError( ERROR_INVALID_HANDLE );
		DEBUGMSG( ZONE_ERROR,
			( TEXT("error: MIXTS Invalid Open Handle: 0x%08x\r\n"), devctx) );
	}

	DEBUGMSG( ZONE_FUNCTION, (TEXT("-MIX_Deinit: %d\r\n"), result) );
	return result;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_PowerDown

Description:

	

Arguments:

	DWORD devctx :

Return Value: void
-------------------------------------------------------------------*/
void MIX_PowerDown( DWORD devctx )
{
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_PowerUp

Description:

	

Arguments:

	DWORD devctx :

Return Value: void
-------------------------------------------------------------------*/
void MIX_PowerUp( DWORD devctx )
{
/*
	if( devctx != (DWORD)g_pDriverContext )
	{
//		SetLastError( ERROR_INVALID_HANDLE );
		DEBUGMSG( ZONE_ERROR,
			( TEXT("error: MIXTS Invalid Open Handle: 0x%08x\r\n"), devctx) );
	}
*/
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Open

Description:

	

Arguments:

	DWORD devctx :
	DWORD access :
	DWORD share :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD MIX_Open( DWORD devctx, DWORD access, DWORD share )
{
	PMIXER_CONTEXT pNewMixer;
	
	DEBUGMSG( ZONE_FUNCTION, (TEXT("+MIX_Open: 0x%08x\r\n"), devctx) );

	if  (devctx == 0)	// opening the device failed; return failure to DEVICE.EXE 
		return 0;
	
	EnterCriticalSection( &g_pDriverContext->mixCritSec );

	if( devctx == (DWORD)g_pDriverContext )
	{
		pNewMixer = (PMIXER_CONTEXT)LocalAlloc(LPTR, sizeof(MIXER_CONTEXT));
		if (!pNewMixer)
		{
			SetLastError( ERROR_OUTOFMEMORY );
		}
		else
		{	// add new mixer to the driver list
			pNewMixer->pNext = g_pDriverContext->pMixerContextList;
			g_pDriverContext->pMixerContextList = pNewMixer;
		}
	}
	else
	{
		DEBUGMSG( ZONE_ERROR, (TEXT("error: MIX_Open: bad Device Handle 0x%08x\r\n"), devctx) );
		SetLastError( ERROR_INVALID_HANDLE );
		pNewMixer = NULL;
	}
	LeaveCriticalSection( &g_pDriverContext->mixCritSec );

	DEBUGMSG( ZONE_FUNCTION, (TEXT("-MIX_Open: 0x%08x\r\n"), pNewMixer) );
	return (DWORD)pNewMixer;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Close

Description:

	

Arguments:

	DWORD openctx :

Return Value: BOOL
-------------------------------------------------------------------*/
BOOL MIX_Close( DWORD openctx )
{
	BOOL retval;
	
	DEBUGMSG( ZONE_FUNCTION, (TEXT("+MIX_Close: 0x%08x\r\n"), openctx) );

	retval = mix_FreeMixer((PMIXER_CONTEXT)openctx);

	DEBUGMSG( ZONE_FUNCTION, (TEXT("-MIX_Close: %d\r\n"), retval) );
	return retval;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Read

Description:

	

Arguments:

	 DWORD openctx :
	void *pBuf :
	DWORD len :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD MIX_Read( DWORD openctx, void *pBuf, DWORD len )
{
	DEBUGMSG( ZONE_FUNCTION, (TEXT("+MIX_Read: 0x%08x, 0x%08x, %d\r\n"), openctx, pBuf, len) );
	return (DWORD)-1;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Seek

Description:

	

Arguments:

	 DWORD openctx :
	long int pos :
	DWORD type :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD MIX_Seek( DWORD openctx, long int pos, DWORD type )
{
	DEBUGMSG( ZONE_ERROR, (TEXT("error: MIX does not seek: 0x%08x\r\n"), openctx) );
	return (DWORD)-1;		// not implemented, -1 indicates error
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_Write

Description:

	

Arguments:

	 DWORD openctx :
	void *pBuf :
	DWORD len :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD MIX_Write( DWORD openctx, void *pBuf, DWORD len )
{
	DEBUGMSG( ZONE_ERROR, (TEXT("error: MIX_Write: MIXTS is read-only: 0x%08x\r\n"), openctx) );
	return (DWORD)-1;		// read-only driver, -1 indicates error
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	MIX_IOControl

Description:

	Handles the calls sent through DevicerIoControl. That's the function that
	communicates with mixapi.lib and does the actual work

Arguments:

	DWORD dwOpenData :	the pointer to the mixer context that the driver returned in MIX_Open
	DWORD dwCode :	must be IOCTL_MIX_MESSAGE
	BYTE  *pBufIn :	pointer to a PMMDRV_MESSAGE_PARAMS that stores the description of the desired action
	DWORD dwLenIn : must be sizeof(MMDRV_MESSAGE_PARAMS)
	BYTE  *pBufOut : stores the return code (DWORD)
	DWORD dwLenOut : if pBufOut is not NULL, that should be at least sizeof(DWORD)
	DWORD *pdwActualOut : if pBufOut is not NULL and pdwActualOut is not NULL,
				that will return sizeof(DWORD)

Return Value: BOOL
-------------------------------------------------------------------*/
BOOL MIX_IOControl(
	DWORD dwOpenData,
	DWORD dwCode,
	BYTE  *pBufIn,
	DWORD dwLenIn,
	BYTE  *pBufOut,
	DWORD dwLenOut,
	DWORD *pdwActualOut )
{
    DWORD dwRet;
    PMMDRV_MESSAGE_PARAMS pParams = (PMMDRV_MESSAGE_PARAMS) pBufIn;
	PDRIVER_CONTEXT pDriverContext = g_pDriverContext;
	PMIXER_CONTEXT pMixer = (PMIXER_CONTEXT)dwOpenData;

	DEBUGMSG( ZONE_TEST|ZONE_FUNCTION, (TEXT("+MIX_IOControl\r\n")));
#if 0
    HEXPARAM(dwOpenData); 
    HEXPARAM(dwCode); 
    HEXPARAM(pBufIn);
    HEXPARAM(dwLenIn); 
    HEXPARAM(pBufOut); 
    HEXPARAM(dwLenOut);
    HEXPARAM(pdwActualOut);
#endif

	ASSERT(pDriverContext);
    //  set the error code to be no error first
    SetLastError(ERROR_SUCCESS);
	if( pdwActualOut != NULL )
		*pdwActualOut = 0L;
	


	if(NULL == pMixer)
	{
		DEBUGMSG( ZONE_ERROR, (TEXT("error: MIX_IOControl bad hOpen\r\n")));
		SetLastError( ERROR_INVALID_HANDLE );
		dwRet = MMSYSERR_INVALHANDLE;
		goto end;
	}
    if (dwCode != IOCTL_MIX_MESSAGE) 
	{
        DEBUGMSG(ZONE_ERROR, (TEXT("Bad parameters in MIX_IOControl()\r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		dwRet = MMSYSERR_INVALPARAM;
		goto end;
    }

	if(pParams->uDeviceId >= MAX_SUPPORTED_INSTANCES) 
	{
        DEBUGMSG(ZONE_ERROR, (TEXT("Bad device ID in MIX_IOControl()\r\n")));
		SetLastError(ERROR_BAD_DEVICE);
		dwRet = MMSYSERR_BADDEVICEID;
		goto end;
    }
	
	
	if (pBufOut != NULL && dwLenOut < sizeof(DWORD)) 
	{
        DEBUGMSG(ZONE_ERROR, (TEXT("Bad return buffer in MIX_IOControl()\r\n")));
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		dwRet = MMSYSERR_INVALPARAM;
		goto end;
	}

	LockMixerList();


    __try 
	{
        switch (pParams->uMsg) 
		{
		case MXDM_GETNUMDEVS:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_GETNUMDEVS\r\n")));
			dwRet = mix_handle_GETNUMDEVS(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_GETDEVCAPS:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_GETDEVCAPS\r\n")));
			dwRet = mix_handle_GETDEVCAPS(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_OPEN:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_OPEN\r\n")));
			dwRet = mix_handle_OPEN(pMixer, &(pParams->dwUser), pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_CLOSE:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_CLOSE\r\n")));
			dwRet = mix_handle_CLOSE(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_GETLINEINFO:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_GETLINEINFO\r\n")));
			dwRet = mix_handle_GETLINEINFO(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_GETLINECONTROLS:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_GETLINECONTROLS\r\n")));
			dwRet = mix_handle_GETLINECONTROLS(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_GETCONTROLDETAILS:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_GETCONTROLDETAILS\r\n")));
			dwRet = mix_handle_GETCONTROLDETAILS(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_SETCONTROLDETAILS:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_SETCONTROLDETAILS\r\n")));
			dwRet = mix_handle_SETCONTROLDETAILS(pMixer, pParams->dwUser, pParams->dwParam1, pParams->dwParam2);
			break;
		case MXDM_INIT:
			DEBUGMSG(ZONE_TEST, (TEXT("MXDM_INIT\r\n")));
			dwRet = MMSYSERR_NOERROR;
			break;
		default:
			DEBUGMSG(ZONE_TEST, (TEXT("Bad code in MIX_IOControl\r\n")));
			dwRet = MMSYSERR_NOTSUPPORTED;
			break;
        }

    } 
	__except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{

        RETAILMSG(1, (TEXT("EXCEPTION IN MIX_IOControl!!!!\r\n")));
		SetLastError(ERROR_GEN_FAILURE);
		dwRet = MMSYSERR_ERROR;
    }   
	
	UnlockMixerList();

end:

    //
    // Pass the return code back via pBufOut
    //
    if (pBufOut != NULL)
	{
		*((DWORD*) pBufOut) = dwRet;
		if (pdwActualOut)
			*pdwActualOut = sizeof(dwRet);
	}

	DEBUGMSG( ZONE_TEST|ZONE_FUNCTION, (TEXT("-MIX_IOControl, dwRet = 0x%8.8x\r\n"), dwRet));
	return 	MMSYSERR_NOERROR == dwRet;
}
