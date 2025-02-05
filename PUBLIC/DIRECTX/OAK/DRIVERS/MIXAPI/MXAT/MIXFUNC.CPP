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

	mixfunc.c

Abstract:

-------------------------------------------------------------------*/
#include <windows.h>
#include "globals.h"
#include "mixdrv.h"
#include "mixfunc.h"
#include "mmreg.h"
#include "csmix.h"
#include "regfake.h"

static void CopyControlStruct(PMIXERCONTROL pMixerControl, int iIndex);


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_GETNUMDEVS

Description:

	Returns the number of supported instances for the driver 
	(1, that is each app can open only one instance). Not that we check for that, 
	i.e, an app could still open multiple instances, but it shouldn't

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 :
	DWORD dwParam2 :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_GETNUMDEVS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	return MAX_SUPPORTED_INSTANCES;	// the driver only supports one device instance at the moment
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_OPEN

Description:

	Opens a mixer handle; i.e., initializes the stored values in the MIXER_CONTEXT structure that
	describes the mixer with the values sent by the application

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD *pdwUser :	given us by the app for storage; we don't use it at this time
	DWORD dwParam1 :	pointer to a MIXEROPENDESC structure, describing parameters
	DWORD dwParam2 :	not used

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_OPEN(PMIXER_CONTEXT pMixer, DWORD *pdwUser, DWORD dwParam1, DWORD dwParam2)
{
	MIXEROPENDESC *pMixDesc = (MIXEROPENDESC *) MapPtrToProcess((PVOID)dwParam1, GetCallerProcess());

	ASSERT(pMixer && pdwUser);

	if (!pMixDesc)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return MMSYSERR_INVALPARAM;
	}
	
	pMixer->dwInstance = pMixDesc->dwInstance;
	pMixer->hProcess = GetCallerProcess();
	pMixer->pfnMixerCallback = (PDRVCALLBACK)pMixDesc->dwCallback;
	*pdwUser = 0;	// not really used
	return MMSYSERR_NOERROR;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_GETDEVCAPS

Description:

	Gets the device capabilities

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 : pointer to a MIXERCAPS struct where the info is returned
	DWORD dwParam2 :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_GETDEVCAPS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	PMIXERCAPS pMixCaps = (PMIXERCAPS) MapPtrToProcess((PMIXERCAPS) dwParam1, GetCallerProcess());
	ASSERT(pMixer);
	if (!pMixCaps)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return MMSYSERR_INVALPARAM;
	}
	// set mixer capabilites
	pMixCaps->wMid = DRIVER_WMID;
	pMixCaps->wPid = MM_CRYSTAL_CS4232_WAVEMIXER;
	pMixCaps->fdwSupport = 0;
	pMixCaps->cDestinations = DESTINATION_LINES;
	pMixCaps->vDriverVersion = DRIVER_VERSION;	// version 1.0
	_tcsncpy(pMixCaps->szPname, MIXERDRIVER_NAME, MAXPNAMELEN);
	pMixCaps->szPname[MAXPNAMELEN - 1] = 0;
	return MMSYSERR_NOERROR;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_CLOSE

Description:

	handles a mixerClose call from mixapi.lib. It does not deallocate the mixer context; that happens
	in MIX_Close, which is called when the calling app calls CloseHandle on the handle returned by
	CreateFile("MIX1:",...)

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 : not used
	DWORD dwParam2 : not used

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_CLOSE(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	ASSERT(pMixer);

	pMixer->dwInstance = 0;
	//pMixer->hMixer = NULL;
	pMixer->pfnMixerCallback = NULL;
	
	return MMSYSERR_NOERROR;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_GETLINEINFO

Description:

	Gets info about a mixer line

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 : pointer to a MIXERLINE struct that gets filled with the line info
	DWORD dwParam2 : flags that describe the info requested

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_GETLINEINFO(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	MIXER_LINES mxLine;
	LPMIXERLINE pTempLine;
	LPMIXERLINE pLine = (LPMIXERLINE) MapPtrToProcess((PVOID)dwParam1, GetCallerProcess());

	ASSERT(pMixer);

	if (!pLine)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLine in mix_handle_GETLINEINFO\r\n")));
		goto badparms;
	}

	if (pLine->cbStruct < sizeof(MIXERLINE) )
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLine cbStruct in mix_handle_GETLINEINFO\r\n")));
		goto badparms;
	}

	// what line does the user want info about?
	
	switch (dwParam2 & MIXER_GETLINEINFOF_QUERYMASK)
	{
	case MIXER_GETLINEINFOF_DESTINATION:
		// the caller wants info about a destination line
		if (pLine->dwSource != 0|| pLine->dwDestination >= MAX_DESTINATION_LINES)
			// be paranoid; the docs say dwSource must needs be 0 in this case
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLine line in mix_handle_GETLINEINFO\r\n")));
			goto badparms;
		}
		mxLine = (MIXER_LINES) pLine->dwDestination;
		// fill in info
 		break;
	case MIXER_GETLINEINFOF_SOURCE:
		// the user wants info about a source line (the one that's associated to destination line pLine->dwDestination
		{
			pTempLine = FindMixerLine(pLine->dwDestination);
			if (!pTempLine)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("Unknown line ID in mix_handle_GETLINEINFO: %d\r\n"), pLine->dwDestination));
				goto badparms;
			}
			if (pLine->dwSource >= pTempLine->cConnections)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLine dwSource (%d) in mix_handle_GETLINEINFO (max %d)\r\n"),
					pLine->dwSource, pTempLine->cConnections));
				goto badparms;
			}
			mxLine = g_Connections[pLine->dwDestination][pLine->dwSource];
			// fill in the info
		}
		break;
	case MIXER_GETLINEINFOF_LINEID:
		if (pLine->dwLineID >= MAX_LINES)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLine dwLineID in mix_handle_GETLINEINFO\r\n")));
			goto badparms;
		}
		mxLine = (MIXER_LINES) pLine->dwLineID;
		break;
	case MIXER_GETLINEINFOF_COMPONENTTYPE:
		{
			// search the first line that supports the type of component requested
			int i; 
			mxLine = NO_CONNECTION;
			for (i = 0; i < MAX_LINES; i++)
			{
				pTempLine = FindMixerLine(i);
				if (!pTempLine)
					continue;
				if (pTempLine->dwComponentType == pLine->dwComponentType)
				{
					mxLine = (MIXER_LINES) i;
					break;
				}
			}
			if (mxLine == NO_CONNECTION)
			{
				return MIXERR_INVALLINE;
			}
		}
		break;
	case MIXER_GETLINEINFOF_TARGETTYPE:
		{
			// search the first line that supports the type of component requested
			int i; 
			mxLine = NO_CONNECTION;
			for (i = 0; i < MAX_LINES; i++)
			{
				pTempLine = FindMixerLine(i);
				if (!pTempLine)
					continue;
				if (pTempLine->Target.dwType == pLine->dwComponentType)
				{
					mxLine = (MIXER_LINES) i;
					break;
				}
			}
			if (mxLine == NO_CONNECTION)
			{
				return MIXERR_INVALLINE;
			}
		}
		break;
	default:
		DEBUGMSG(ZONE_ERROR, (TEXT("Unknown param in mix_handle_GETLINEINFO: %d\r\n"), dwParam2));
		return MMSYSERR_NOTSUPPORTED;
	}

	pTempLine = FindMixerLine(mxLine);
	pLine->dwLineID = pTempLine->dwLineID;
	pLine->dwComponentType = pTempLine->dwComponentType;
	pLine->cChannels = pTempLine->cChannels;
	pLine->cConnections = pTempLine->cConnections;
	pLine->cControls = pTempLine->cControls;
	pLine->dwDestination = pTempLine->dwDestination;
	pLine->dwSource = pTempLine->dwSource;
	_tcscpy(pLine->szShortName, pTempLine->szShortName);
	_tcscpy(pLine->szName, pTempLine->szName);

	pLine->Target.dwType = pTempLine->Target.dwType;
	pLine->Target.dwDeviceID = pTempLine->Target.dwDeviceID;
	pLine->Target.wMid = pTempLine->Target.wMid;
	pLine->Target.wPid = pTempLine->Target.wPid;
	pLine->Target.vDriverVersion = pTempLine->Target.vDriverVersion;
	pLine->fdwLine = pTempLine->fdwLine;
	_tcscpy(pLine->Target.szPname, pTempLine->Target.szPname);

	return MMSYSERR_NOERROR;

badparms:
	DEBUGMSG(ZONE_ERROR, (TEXT("Bad parameters in mix_handle_GETLINEINFO\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return MMSYSERR_INVALPARAM;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_GETLINECONTROLS

Description:

	Gets info about the controls of a line

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 : pointer to a MIXERLINECONTROLS structure that is filled with the controls info
	DWORD dwParam2 : flags that describe the information requested

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_GETLINECONTROLS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPMIXERLINECONTROLS pLineControl = (LPMIXERLINECONTROLS)MapPtrToProcess((PVOID)dwParam1, 
						GetCallerProcess());
	LPMIXERCONTROL pMixerControl;
	DWORD i;

	ASSERT(pMixer);

	if (!pLineControl)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLineControl in mix_handle_GETLINECONTROLS\r\n")));
		goto badparms;
	}
	pMixerControl = (LPMIXERCONTROL)MapPtrToProcess(pLineControl->pamxctrl, GetCallerProcess());

	if (pLineControl->cbStruct < sizeof(MIXERLINECONTROLS) || pLineControl->cControls < 1 || pMixerControl == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Bad pMixerControl in mix_handle_GETLINECONTROLS\r\n")));
		goto badparms;
	}
	
	// first identify the control

	switch (dwParam2 & MIXER_GETLINECONTROLSF_QUERYMASK)
	{
	case MIXER_GETLINECONTROLSF_ALL:
		{
			int nControls;
			LPMIXERLINE pTempLine;
			DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETLINECONTROLS: MIXER_GETLINECONTROLSF_ALL\r\n")));
			//the caller wants all of this line's controls
			if (pLineControl->dwLineID >= MAX_LINES)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("Bad pLineControl dwLineID in mix_handle_GETLINECONTROLS\r\n")));
				return MIXERR_INVALLINE;
			}
			
			// does the line have any controls?
			pTempLine = FindMixerLine(pLineControl->dwLineID);
			if (!pTempLine)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("Unknown pLineControl dwLineID in mix_handle_GETLINECONTROLS\r\n")));
				return MIXERR_INVALLINE;
			}
			if (pTempLine->cControls == 0 || 
				pTempLine->cControls > pLineControl->cControls)
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("Line has no controls in mix_handle_GETLINECONTROLS\r\n")));
				return MIXERR_INVALLINE;
			}
			
			// all seems ok; let's find the first line control
			for (nControls = i = 0; i < sizeof(g_Controls)/sizeof(g_Controls[0]); i++)
			{
				if (g_Controls[i].dwLineID == pLineControl->dwLineID)
				{
					CopyControlStruct(&pMixerControl[nControls++], i);
				}
			}
		}
		break;
	case MIXER_GETLINECONTROLSF_ONEBYID:
		DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETLINECONTROLS: MIXER_GETLINECONTROLSF_ONEBYID: %d\r\n"), pLineControl->dwControlID));
		if (pLineControl->dwControlID >= sizeof(g_Controls)/sizeof(g_Controls[0]))
			return MIXERR_INVALCONTROL;
		// the ID is the index of the control in the g_Controls table;
		if (pLineControl->cControls != 1)
			goto badparms;
		CopyControlStruct(pMixerControl, pLineControl->dwControlID);
		break;
	case MIXER_GETLINECONTROLSF_ONEBYTYPE:
		DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETLINECONTROLS: MIXER_GETLINECONTROLSF_ONEBYTYPE: line %d, type %d\r\n"), 
			pLineControl->dwLineID, pLineControl->dwControlType));
		if (pLineControl->cControls != 1)
			goto badparms;
		for (i = 0; i < sizeof(g_Controls)/sizeof(g_Controls[0]); i++)
		{
			if (g_Controls[i].dwLineID == pLineControl->dwLineID && 
				(g_Controls[i].dwControlType & MIXERCONTROL_CT_CLASS_MASK) == 
					(pLineControl->dwControlType & MIXERCONTROL_CT_CLASS_MASK))
			{
				CopyControlStruct(pMixerControl, i);
				break;
			}
		}
		// did we find any?
		if (i == sizeof(g_Controls)/sizeof(g_Controls[0]))
		{
			// no
			DEBUGMSG(ZONE_ERROR, (TEXT("No control of requested type\r\n")));
			return MIXERR_INVALCONTROL;
		}
		break;
	default:
		return MMSYSERR_INVALFLAG;
	}

	return MMSYSERR_NOERROR;

badparms:
	DEBUGMSG(ZONE_ERROR, (TEXT("Bad parameters in mix_handle_GETLINECONTROLS\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return MMSYSERR_INVALPARAM;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_GETCONTROLDETAILS

Description:

	Gets detailed information about a control

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 : pointer to the MIXERCONTROLDETAILS structure where the function returns info
	DWORD dwParam2 : flags that describe the information requested

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_GETCONTROLDETAILS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	LPMIXERCONTROLDETAILS pControlDetails = (LPMIXERCONTROLDETAILS) MapPtrToProcess((PVOID)dwParam1, GetCallerProcess());
	DWORD dwRet = NO_ERROR;

	PDRIVER_CONTEXT pDriverContext = g_pDriverContext;
	ASSERT(pDriverContext);

	if (!pControlDetails)
		goto badparms;

	if (pControlDetails->cbStruct < sizeof(MIXERCONTROLDETAILS))
		goto badparms;
	if (pControlDetails->dwControlID >= MAX_CONTROLS)
	{
		DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETCONTROLDETAILS: Bad control ID: %d\r\n"),
			pControlDetails->dwControlID));

		return MIXERR_INVALCONTROL;
	}

	switch (dwParam2 & MIXER_GETCONTROLDETAILSF_QUERYMASK)
	{
	case MIXER_GETCONTROLDETAILSF_VALUE:
		DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETCONTROLDETAILS: MIXER_GETCONTROLDETAILSF_VALUE: control %d\r\n"),
			pControlDetails->dwControlID));

		{
			DWORD dwRet;
			switch(g_Controls[pControlDetails->dwControlID].dwControlType)
			{
			case MIXERCONTROL_CONTROLTYPE_MUX:
			case MIXERCONTROL_CONTROLTYPE_MUTE:
				{
					MIXERCONTROLDETAILS_BOOLEAN *pDetails = (MIXERCONTROLDETAILS_BOOLEAN *)
						MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
					
					if (!pDetails || pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_BOOLEAN))
					{
						DEBUGMSG(ZONE_ERROR, (TEXT("mix_handle_GETCONTROLDETAILS: Bad parms: pointer: 0x%8.8x, size %d\r\n"),
							pDetails, pControlDetails->cbDetails));
						
						goto badparms;
					}
					
					if (IsBadWritePtr((LPVOID)pDetails, sizeof(MIXERCONTROLDETAILS_BOOLEAN)))
					{
						DEBUGMSG(ZONE_ERROR, (TEXT("error: Bad pointer in mix_handle_GETCONTROLDETAILS\r\n")));
						goto badparms;
					}
					
					if (g_Controls[pControlDetails->dwControlID].dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
					{
						DWORD fValue;
						dwRet = GetMuxValue(pDriverContext,	pControlDetails->dwControlID, &fValue);
						pDetails->fValue = fValue;
						
					}
					else	// mute
					{
						int fValue;
						dwRet = GetMixerMute(pDriverContext, pControlDetails->dwControlID, &fValue);
						pDetails->fValue = fValue;
					}
				}
				break;
			case MIXERCONTROL_CONTROLTYPE_VOLUME:
				{
					MIXERCONTROLDETAILS_UNSIGNED *pDetails = (MIXERCONTROLDETAILS_UNSIGNED *)
						MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
					
					// for UNIFORM controls (in our case, MUTE and MUX), there needs be only one 
					// MIXERCONTROLDETAILS_xxx structure; for non uniform controls
					// we need one structure for each channel
					
					if (!pDetails || pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_UNSIGNED))
						goto badparms;

					if (IsBadWritePtr((LPVOID)pDetails,  pControlDetails->cChannels * pControlDetails->cbDetails))
					{
						ERRORMSG(1, (TEXT("error: Bad pointer in mix_handle_GETCONTROLDETAILS\r\n")));
						goto badparms;
					}
					
					dwRet = GetMixerVolume(pDriverContext,
						pControlDetails->dwControlID,
						&(pDetails[LEFT_CHANNEL].dwValue), &(pDetails[RIGHT_CHANNEL].dwValue));

					// scale values (if scaling window handle set; see scalevol.h for details)

				}
				break;
			case MIXERCONTROL_CONTROLTYPE_UNSIGNED:	// only one of that type: the ZV port sample rate
				{
					MIXERCONTROLDETAILS_UNSIGNED *pDetails = (MIXERCONTROLDETAILS_UNSIGNED *)
						MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
					
					if (!pDetails || pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_UNSIGNED))
						goto badparms;

					if (IsBadWritePtr((LPVOID)pDetails, sizeof(MIXERCONTROLDETAILS_UNSIGNED)))
					{
						DEBUGMSG(ZONE_ERROR, (TEXT("error: Bad pointer in mix_handle_GETCONTROLDETAILS\r\n")));
						goto badparms;
					}
					
					dwRet = GetUnsignedValue(pDriverContext,
						pControlDetails->dwControlID,
						&(pDetails->dwValue));
				}
				break;
			default:
				DEBUGMSG(ZONE_ERROR, (TEXT("mix_handle_GETCONTROLDETAILS: Bad control type: %d\r\n"),
				g_Controls[pControlDetails->dwControlID].dwControlType));

				ASSERT(FALSE);
			}
		}
		break;
	case MIXER_GETCONTROLDETAILSF_LISTTEXT:
		DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETCONTROLDETAILS: MIXER_GETCONTROLDETAILSF_LISTTEXT: control %d\r\n"),
			pControlDetails->dwControlID));
		switch(g_Controls[pControlDetails->dwControlID].dwControlType)
		{
		case MIXERCONTROL_CONTROLTYPE_MUX:
			{
				int i;
				MIXERCONTROLDETAILS_LISTTEXT *pDetails = (MIXERCONTROLDETAILS_LISTTEXT *)
					MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
				
				if (!pDetails || pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_LISTTEXT))
					goto badparms;

				if (IsBadWritePtr(pDetails->szName, MIXER_LONG_NAME_CHARS))
				{
					DEBUGMSG(ZONE_ERROR, (TEXT("error: Bad pointer in mix_handle_GETCONTROLDETAILS\r\n")));
					goto badparms;
				}

				// get the current mux'ed line ID and name
				for (i = 0; i < MAX_MULTIPLEXERS; i++)
				{
					if (g_Multiplexers[i].dwMuxID == pControlDetails->dwControlID)
					{
						DWORD dwValue;
						LPMIXERLINE pTempLine;
						dwRet = GetMuxValue(pDriverContext, pControlDetails->dwControlID, &dwValue);
						if (NO_ERROR != dwRet)
						{
							SetLastError(dwRet);
							return MIXERR_INVALCONTROL; // not supported or another error
						}

						pDetails->dwParam1 = g_Multiplexers[i].muxedLines[dwValue];
						pTempLine = FindMixerLine(pDetails->dwParam1);
						if (!pTempLine)
						{
							DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_GETCONTROLDETAILS: Bad line ID: %d\r\n"),
								pDetails->dwParam1));
							return MIXERR_INVALLINE;
						}
						_tcscpy(pDetails->szName, pTempLine->szName);
						break;
					}
				}
				ASSERT(i < MAX_MULTIPLEXERS);
			}
			break;
		default:	// can't do MIXER_GETCONTROLDETAILSF_LISTTEXT on volume or mute controls
			goto badparms;
		}
		break;
	default:
		return MMSYSERR_INVALFLAG;
	}

	if (dwRet != NO_ERROR)
	{
		SetLastError(dwRet);
		return MMSYSERR_NODRIVER;
	}

	return MMSYSERR_NOERROR;

badparms:
	DEBUGMSG(ZONE_ERROR, (TEXT("Bad parameters in mix_handle_GETCONTROLDETAILS\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return MMSYSERR_INVALPARAM;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_handle_SETCONTROLDETAILS

Description:

	Sets a control's value

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure
	DWORD dwUser : returned by the app (not used)
	DWORD dwParam1 : pointer to the MIXERCONTROLDETAILS structure describing the control
	DWORD dwParam2 : flags that describe the value to be set

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD mix_handle_SETCONTROLDETAILS(PMIXER_CONTEXT pMixer, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
	// this is where the real work is!
	LPMIXERCONTROLDETAILS pControlDetails = (LPMIXERCONTROLDETAILS) MapPtrToProcess((PVOID)dwParam1, GetCallerProcess());
	PDRIVER_CONTEXT pDriverContext = g_pDriverContext;
	ASSERT(pMixer && pDriverContext);

	if (!pControlDetails)
		goto badparms;

	if (pControlDetails->cbStruct < sizeof(MIXERCONTROLDETAILS))
		goto badparms;

	if (pControlDetails->dwControlID >= MAX_CONTROLS)
	{
		DEBUGMSG(ZONE_PARAMS, (TEXT("mix_handle_SETCONTROLDETAILS: Bad control ID: %d\r\n"),
			pControlDetails->dwControlID));
		return MIXERR_INVALCONTROL;
	}

	switch (dwParam2 & MIXER_SETCONTROLDETAILSF_QUERYMASK )
	{
	case MIXER_SETCONTROLDETAILSF_VALUE:
		{
			DWORD dwRet;
			switch(g_Controls[pControlDetails->dwControlID].dwControlType)
			{
			case MIXERCONTROL_CONTROLTYPE_MUX:
			case MIXERCONTROL_CONTROLTYPE_MUTE:
				{
					MIXERCONTROLDETAILS_BOOLEAN *pDetails = (MIXERCONTROLDETAILS_BOOLEAN *)
						MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
					
					if (!pDetails || pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_BOOLEAN))
						goto badparms;
					
					if (g_Controls[pControlDetails->dwControlID].dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
					{
						dwRet = SetMuxValue(pDriverContext,
							pControlDetails->dwControlID, 
							pDetails->fValue);
						
					}
					else	// mute
					{
						dwRet = SetMixerMute(pDriverContext, 
							pControlDetails->dwControlID,
							pDetails->fValue);
					}
				}
				break;
			case MIXERCONTROL_CONTROLTYPE_VOLUME:
				{
					MIXERCONTROLDETAILS_UNSIGNED *pDetails = (MIXERCONTROLDETAILS_UNSIGNED *)
						MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
					
					// for UNIFORM controls (in our case, MUTE and MUX), there needs be only one 
					// MIXERCONTROLDETAILS_xxx structure; for non uniform controls
					// we need one structure for each channel

					if (!pDetails || 
						pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_UNSIGNED))
						goto badparms;

					if (IsBadWritePtr((LPVOID)pDetails,  pControlDetails->cChannels * pControlDetails->cbDetails))
					{
						ERRORMSG(1, (TEXT("error: Bad pointer in mix_handle_SETCONTROLDETAILS\r\n")));
						goto badparms;
					}

					if (pControlDetails->cChannels == 1)
						pDetails[RIGHT_CHANNEL].dwValue = pDetails[LEFT_CHANNEL].dwValue;

					if (pDetails[LEFT_CHANNEL].dwValue > MAX_ATTENUATION ||
							pDetails[RIGHT_CHANNEL].dwValue > MAX_ATTENUATION)
						goto badparms;

					dwRet = SetMixerVolume(pDriverContext,
						pControlDetails->dwControlID,
						pDetails[LEFT_CHANNEL].dwValue, pDetails[RIGHT_CHANNEL].dwValue);

				}
				break;
			case MIXERCONTROL_CONTROLTYPE_UNSIGNED:	// only one of that type: the ZV port sample rate
				{
					MIXERCONTROLDETAILS_UNSIGNED *pDetails = (MIXERCONTROLDETAILS_UNSIGNED *)
						MapPtrToProcess(pControlDetails->paDetails, GetCallerProcess());
					
					if (!pDetails || pControlDetails->cbDetails < sizeof(MIXERCONTROLDETAILS_UNSIGNED))
						goto badparms;

					dwRet = SetUnsignedValue(pDriverContext,
						pControlDetails->dwControlID,
						pDetails->dwValue);
				}
				break;
			default:
				RETAILMSG(1, (TEXT("error: Control %d, unknown type: 0x%8.8x\r\n"), pControlDetails->dwControlID, 
					g_Controls[pControlDetails->dwControlID].dwControlType));
				dwRet = ERROR_NOT_SUPPORTED;
			}
			
			if (dwRet != NO_ERROR)
			{
				SetLastError(dwRet);
				return MMSYSERR_NODRIVER;
			}
		}
		break;
	default:
		return MMSYSERR_INVALFLAG;
	}

	// all seems ok; call registered callbacks (if any)

	{
		PMIXER_CONTEXT pMix = pDriverContext->pMixerContextList;
		CALLBACKINFO cbi;
		DWORD dwOldPermissions = SetProcPermissions(0xFFFFFFFF);
		DEBUGMSG(ZONE_VERBOSE, (TEXT("Old permissions = 0x%8.8X\r\n"), dwOldPermissions));
		while (pMix)
		{
			if (pMix->pfnMixerCallback)
			{
				cbi.hProc   = (PVOID) pMix->hProcess;
				cbi.pfn     = (FARPROC) pMix->pfnMixerCallback;
				cbi.pvArg0  = (PVOID) pMix;	// the first parameter is here
				DEBUGMSG(ZONE_VERBOSE, (TEXT("Sending callback: 0x%8.8X, permissions = 0x%8.8X\r\n"), 
					pMix->pfnMixerCallback,
					GetCurrentPermissions()));
				__try
				{
					PerformCallBack(&cbi, MM_MIXM_CONTROL_CHANGE, pMix->dwInstance, pControlDetails->dwControlID, 0);
				} 
				__except(EXCEPTION_EXECUTE_HANDLER) 
				{
					ERRMSG("bad callback!");
				}
			}
			pMix = pMix->pNext;
		}
	    SetProcPermissions(dwOldPermissions );
	}
	DEBUGMSG(ZONE_VERBOSE, (TEXT("Callback done\r\n")));
	return MMSYSERR_NOERROR;

badparms:
	DEBUGMSG(ZONE_ERROR, (TEXT("Bad parameters in mix_handle_GETCONTROLDETAILS\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return MMSYSERR_INVALPARAM;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	CopyControlStruct

Description:

	

Arguments:

	PMIXERCONTROL pMixerControl :
	int iIndex :

Return Value: void
-------------------------------------------------------------------*/
void CopyControlStruct(PMIXERCONTROL pMixerControl, int iIndex)
{
	DEBUGMSG(ZONE_FUNCTION|ZONE_VERBOSE, (TEXT("+CopyControlStruct index %d\r\n"), iIndex));

	pMixerControl->cbStruct = sizeof(MIXERCONTROL);
	pMixerControl->dwControlID = iIndex;
//	pMixerControl->dwControlType = g_Controls[iIndex].dwControlType & (MIXERCONTROL_CT_SUBCLASS_MASK | MIXERCONTROL_CT_CLASS_MASK); 
	pMixerControl->dwControlType = g_Controls[iIndex].dwControlType; 
	// the volume controls act separately on the two channels (i.e., you can have different volumes on left and right)
	// the mute and mux are uniform: they act the same on both channels
	pMixerControl->fdwControl = (MIXERCONTROL_CONTROLTYPE_VOLUME == g_Controls[iIndex].dwControlType)?
								0:
								MIXERCONTROL_CONTROLF_UNIFORM; 
	pMixerControl->cMultipleItems = 0; 
	_tcscpy(pMixerControl->szShortName, g_Controls[iIndex].szShortName);
	_tcscpy(pMixerControl->szName, g_Controls[iIndex].szName); 
//	pMixerControl->Bounds.dwMinimum = g_Controls[iIndex].dwMinimum; 
//	pMixerControl->Bounds.dwMaximum = g_Controls[iIndex].dwMaximum;
	
	// [CC - 7/23/1999 ] see comments in csmix.h
	pMixerControl->Bounds.dwMinimum = 0; 
	pMixerControl->Bounds.dwMaximum = MAX_ATTENUATION;
	pMixerControl->Metrics.cSteps = MAX_ATTENUATION;
	DEBUGMSG(ZONE_FUNCTION|ZONE_VERBOSE, (TEXT("-CopyControlStruct\r\n")));
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_InitMixer

Description:

	initialize the fake hardware mixer to a default state, that allows the wave driver to behave correctly (i.e., play)
	without having the user fiddle with the mix device
	

Arguments:

	PMIXER_CONTEXT pMixer :

Return Value: void
-------------------------------------------------------------------*/
void mix_InitMixer( PDRIVER_CONTEXT pDriverContext )
{
	// set all multiplexers
	// initialize GPIO 
	// switch the mixer output to line out
	DEBUGMSG(ZONE_FUNCTION|ZONE_VERBOSE, (TEXT("+mix_InitMixer\r\n")));

	SetMuxValue(pDriverContext, 1/*LINE_OUT_MUX*/, 1);	// line out from mixer
	SetMuxValue(pDriverContext, 2/*RF_OUT_MUX*/, 1);	// RF out from mixer
	
	// disable mute all
	SetMixerMute(pDriverContext, 3 /*LINE_OUT_MUTE_ALL*/, UNMUTE);
	SetMixerMute(pDriverContext, 4 /*SPDIF_OUT_MUTE*/, MUTE);
	
	// set all volume/mute registers

	// master
	WriteAC97 ( pDriverContext, BA0_AC97_MASTER_VOLUME, 0);

	// PCM Out
	WriteAC97 ( pDriverContext, BA0_AC97_PCM_OUT_VOLUME, 0);      // max PCM out(clip)

	// and AC97 headphone volume
	WriteAC97 ( pDriverContext, BA0_AC97_HEADPHONE_VOLUME, 0);      // max

	DEBUGMSG(ZONE_FUNCTION|ZONE_VERBOSE, (TEXT("-mix_InitMixer\r\n")));
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	mix_FreeMixer

Description:

	Frees a mixer device context and removes it from the contexts list

Arguments:

	PMIXER_CONTEXT pMixer :  pointer to the mixer context structure

Return Value: BOOL
-------------------------------------------------------------------*/
BOOL mix_FreeMixer(PMIXER_CONTEXT pMixer)
{
	BOOL retval = TRUE;
	PMIXER_CONTEXT pTemp, pPrev = NULL;
	
	LockMixerList();
	pTemp = g_pDriverContext->pMixerContextList;

	while(pTemp)
	{
		if (pTemp == pMixer)
			break;
		pPrev = pTemp;
		pTemp = pTemp->pNext;
	}
	
	if (NULL == pTemp)
	{	// the device wasn't found in the list
		DEBUGMSG( ZONE_ERROR, (TEXT("Error: mix_FreeMixer: bad Open Handle 0x%08x\r\n"), pMixer) );
		SetLastError( ERROR_INVALID_HANDLE );
		retval = FALSE;
	}
	else
	{	// device found, pTemp points to it, and pPrev points to the previous one in the list
		// unless it's the first one, in which case pPrev is null
		if (pPrev)
			pPrev->pNext = pTemp->pNext;
		else
			g_pDriverContext->pMixerContextList = pTemp->pNext;

		LocalFree(pTemp);
	}

	UnlockMixerList();

	return retval;
}



#if 0
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	ScaleValues

Description:

	Scales the values to be transmitted to the hardware
	Currently calls the GI tv application to do the work;
	this may change in the future
	
Arguments:

	PMIXER_CONTEXT pMixer :	the driver context
	DWORD dwControlIndex :				the index in the g_Controls array of the control to be scaled
	DWORD *pdwLeft :					the 
	DWORD *pdwRight :
	UMSG bDirectConversion :

Return Value: void
-------------------------------------------------------------------*/
void ScaleValues(PMIXER_CONTEXT pMixer, DWORD dwControlIndex,
				 DWORD *pdwLeft, DWORD *pdwRight, BOOL bDirectConversion)
{
	// BUGBUG is scaling to be done for all channels? It makes sense for TV audio, but not for others
	/*
	
	  if (dwControlIndex != 0)
		return;
	*/
	if (IsWindow(pDriverContext->hHookWnd))
	{
		DWORD dwVal = SendMessage(pDriverContext->hHookWnd, // the window of the GI tv app
			bDirectConversion?	
				WM_SCALEDIRECT:			// convert from application data to hardware data
				WM_SCALEINVERSE,		// convert from hardware to app data
			MAKELONG(g_Controls[dwControlIndex].dwMinimum,
			g_Controls[dwControlIndex].dwMaximum),	// minimum and maximum values the hardware accepts for the control
			MAKELONG(*pdwRight, *pdwLeft));	// values to be scaled

		// check the received values against the bounds
		if (GET_LEFT_CHANNEL(dwVal) >= min(g_Controls[dwControlIndex].dwMinimum,
				g_Controls[dwControlIndex].dwMaximum) &&
			GET_LEFT_CHANNEL(dwVal) <= max(g_Controls[dwControlIndex].dwMinimum,
				g_Controls[dwControlIndex].dwMaximum) &&
			GET_RIGHT_CHANNEL(dwVal) >= min(g_Controls[dwControlIndex].dwMinimum,
				g_Controls[dwControlIndex].dwMaximum) &&
			GET_RIGHT_CHANNEL(dwVal) <= max(g_Controls[dwControlIndex].dwMinimum,
				g_Controls[dwControlIndex].dwMaximum))
		{	// Ok, set those values
			*pdwRight = GET_RIGHT_CHANNEL(dwVal);
			*pdwLeft = GET_LEFT_CHANNEL(dwVal);

			DEBUGMSG(ZONE_VERBOSE, (TEXT("ScaleValues: Got scaled values: (%d, %d)\r\n"),
					*pdwLeft,
					*pdwRight));
		}
		else
		{	// received values are bad, ignore them
			DEBUGMSG(ZONE_ERROR, (TEXT("ScaleValues: Got bad scaled values: (%d, %d)\r\n"),
					GET_LEFT_CHANNEL(dwVal),
					GET_RIGHT_CHANNEL(dwVal)));
		}
	}
}
#endif

