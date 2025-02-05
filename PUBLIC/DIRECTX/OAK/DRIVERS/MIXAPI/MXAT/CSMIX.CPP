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

	csmix.c

Abstract:

	Defines the structures and functions that describe the fake mixer hardware

-------------------------------------------------------------------*/

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>


#include "globals.h"
#include "mixdrv.h"
#include "regfake.h"
#include "fakehw.h"
#include "csmix.h"

DWORD GetAssociatedMuteID(DWORD dwVolumeControlID);

// indexes for the audio sources of the platform
// all of them can be accessed as lines by users of mixapi.lib

/*------------------------------------------------------------------------------
	All these arrays have to be initialized with the corresponding 
	indexes for the audio sources of the platform
-------------------------------------------------------------------------------*/

MIXERLINE g_mixerLines[MAX_LINES] ;


// the g_Connections array maps the possible hardware connections between the input lines
// (sources) and the destination lines; the position in the array is the index of the 
// connection for the destination line

MIXER_LINES g_Connections[MAX_DESTINATION_LINES][MAX_LINES - MAX_DESTINATION_LINES - 1];


// Controls

//	Note! the "minimum" value in the following array means the one that sets the volume to minimum
//	that's a different meaning from the one of the logical value exposed to applications!

SHORTMIXERCONTROL g_Controls[MAX_CONTROLS] ;

// only valid for volume and mute controls;
// stores pairs of associated controls (mute and volume, used to mute
// a line when the volume is less than the hardware min)
DWORD g_ControlsPairs[5][2] ;

// that structure describes the lines that are multiplexed
MULTIPLEXER g_Multiplexers[MAX_MULTIPLEXERS] ;


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	SetMixerVolume

Description:

	Sets the volume for one of the mixer controls

Arguments:

	PDRIVER_CONTEXT pDriverContext : the context
	DWORD dwId :	the ID of the control
	DWORD dwLeft :	the new volume for the left channel
	DWORD dwRight :	the new volume for the right channel

Return Value: DWORD
	NO_ERROR : the op succeeded
-------------------------------------------------------------------*/
DWORD SetMixerVolume(PDRIVER_CONTEXT pDriverContext, DWORD dwId, DWORD dwLeft, DWORD dwRight )
{
	DWORD wVal, dwAddr, wMuted = 0;
	DWORD dwRet = NO_ERROR;
		
	ASSERT(pDriverContext && dwId >= 0 && dwId < MAX_CONTROLS);

	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+SetMixerVolume id: %d - %s (%d, %d)\r\n"),
		dwId, g_Controls[dwId].szShortName, dwLeft, dwRight ));

	dwAddr = g_Controls[dwId].dwRegister; 
	g_Controls[dwId].dwLogicalValue = MAKELONG(dwLeft, dwRight);


	switch (dwId)
	{
	case 1 /*ZV_PORT_VOLUME*/:	// that's a 16 bit volume control, and doesn't use a mute flag
		// scale: the min value is 0x8000 (for 0 dB), the max is 0xFFFF (for -85 db)
		// Get the hardware value

		{
			if (dwLeft > 85000L)
			{
				dwLeft = 85000L;
			}
			if (dwRight > 85000L)
			{
				dwRight = 85000L;
			}
		
			dwLeft = g_Controls[dwId].dwMinimum + (g_Controls[dwId].dwMaximum - g_Controls[dwId].dwMinimum)*(dwLeft/85000L);
			dwRight = g_Controls[dwId].dwMinimum + (g_Controls[dwId].dwMaximum - g_Controls[dwId].dwMinimum)*(dwRight/85000L);
		}

		HrdwSetVolume(pDriverContext, dwAddr, MAKELONG(dwLeft, dwRight));
		break;
	// the mic input needs special handling

	case 2 /*BCM_MIXED_VOLUME*/:
		// the logical range of the mic gain is from +32 dB to -34.5 dB, in steps of 1.5 dB
		// the physical range of the variable amplifier is +12 dB to -34.5 dB
		// we map the logical range to values between 0 (representing +32 dB) and 0x2C
		// (representing -34.5 dB)

		// each hardware bit represents 1.5 db, so we don't need any more scaling
		
		dwRight /= 1500;	// transform from hundredths of db attenuation to hardware bits

		// find out whether the amplifier is on

		if (dwRight <= MIXDEV_MIC_AMP_GAIN)
			// yes;
		{
			DEBUGMSG(ZONE_TEST|ZONE_VERBOSE, (TEXT("Turning Mic amp on\r\n")));
			dwRight |= MIXDEV_MIC_AMP_ON;
		}
		else
		{
			DEBUGMSG(ZONE_TEST|ZONE_VERBOSE, (TEXT("Turning Mic amp off\r\n")));
			dwRight -= MIXDEV_MIC_AMP_GAIN;
			dwRight &= ~MIXDEV_MIC_AMP_ON;
		}
		dwLeft = 0;
		// and fall through to actually set the volume
	case 3 /*LINE_OUT_VOLUME*/:
	case 4 /*RF_OUT_VOLUME*/:
	case 5 /*LINE_IN_MIXED_VOLUME*/:
	case 6 /*PCM_VOLUME*/:
	case 7 /*BTSC_VOLUME*/:
	case 8 /*SPDIF_OUT_VOLUME*/:
		
		dwRet = PeekBA0 ( pDriverContext, dwAddr, &wMuted );
		if( dwRet == NO_ERROR )
		{
			BOOL bMute = FALSE;
			// separate mute flag
			wMuted &= MIXDEV_MIXER_MUTE;

			// is the attenuation higher than the hardware range?
			if (dwLeft > g_Controls[dwId].dwMinimum)
			{
				bMute = TRUE;
				dwLeft = g_Controls[dwId].dwMinimum;
			}

			if (dwRight > g_Controls[dwId].dwMinimum)
			{
				bMute = TRUE;
				dwRight = g_Controls[dwId].dwMinimum;
			}
			
			if (bMute)	// hack-mute
			{
				DEBUGMSG(ZONE_TEST, (TEXT("Hack-muting!!!\r\n")));
				wMuted |= MIXDEV_MIXER_MUTE;
			}
			else if (wMuted) 
			{	
				// muted, but volume set to something large: need to know if this was really mute, or 
				// transparently muted by the driver during a previous volume setting
				
				DWORD dwMute = GetAssociatedMuteID(dwId);
				if (dwMute != (DWORD)-1)
				{
					if (!g_Controls[dwMute].dwLogicalValue)
					{
						DEBUGMSG(ZONE_TEST, (TEXT("Hack detected! Unmuting!!!\r\n")));
						wMuted = 0;
					}
#ifdef DEBUG
					else
					{
						DEBUGMSG(ZONE_TEST, (TEXT("Real mute detected; pass on!!!\r\n")));
					}
#endif
				}
			}

			wVal = (dwLeft << 8) | dwRight | wMuted;
			DEBUGMSG(ZONE_TEST, (TEXT("SetMixerVolume dwLeft = %d, dwRight = %d, wMuted = 0x%08x\r\n"), dwLeft, dwRight, wMuted));
			DEBUGMSG(ZONE_TEST, (TEXT("SetMixerVolume PokeBA0: 0x%08x\r\n"), wVal));
			dwRet = PokeBA0 ( pDriverContext,  dwAddr, wVal );
		}

		DEBUGMSG((dwRet == NO_ERROR)?0:ZONE_ERROR, (TEXT("error: SetMixerVolume PeekBAO read fail: 0x%08x\r\n"), dwRet) );

		break;
	default:
		//error: we received an ID for a control that's not a volume!
		ASSERT(FALSE);
		DEBUGMSG( ZONE_ERROR, (TEXT("error: SetMixerVolume: bad ID: %d\r\n"), dwId));
		dwRet = ERROR_NOT_SUPPORTED;
	}

	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST, (TEXT("-SetMixerVolume: 0x%08x\r\n"), dwRet) );
	return dwRet;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	SetMixerMute

Description:

	Sets muting for one of the mixer controls. Only works with MUTE controls

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD dwId :
	BOOL bMute :	TRUE: mute line; FALSE: unmute line

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD SetMixerMute(PDRIVER_CONTEXT pDriverContext, DWORD dwId, BOOL bMute)
{

	DWORD dwVal, dwAddr;
	DWORD dwRet = NO_ERROR;


	ASSERT(pDriverContext && dwId >= 0 && dwId < MAX_CONTROLS);

	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+SetMixerMute id %d -%s:  %s\r\n"), 
		dwId, g_Controls[dwId].szShortName, bMute?TEXT("MUTE"):TEXT("UNMUTE")) );
	
	dwAddr = g_Controls[dwId].dwRegister;
	g_Controls[dwId].dwLogicalValue = bMute;
	
	if (dwId != 1 /*ZV_PORT_MUTE*/)
	{
		dwRet = PeekBA0 ( pDriverContext,  dwAddr, &dwVal );

		if( dwRet != NO_ERROR )
		{
			DEBUGMSG( ZONE_ERROR, (TEXT("error: SetMixerMute PeekBAO read fail: 0x%08x\r\n"), dwRet) );
			return dwRet;
		}
	}

	switch (dwId)
	{
	case 1 /*ZV_PORT_MUTE*/:
		// It's the ZV port mute; special processing 
		
		DEBUGMSG(ZONE_TEST|ZONE_VERBOSE, (TEXT("Muting ZV Port!\r\n")));
		
		pDriverContext->bZVMute = bMute;
		
		if (bMute)	// ? BUGBUG verify that! As of today (Jun 17) there is no info on bits assignments in the 4620!
		{
			PokeIO(pDriverContext, IO_ASER1_CONTROL, 0);
		}
		else
		{
			PokeIO(pDriverContext, IO_ASER1_CONTROL, ASER1_CONTROL_ME);
		}
		
		break;
	case 2 /*LINE_OUT_MUTE_ALL*/:
	case 3 /*RF_OUT_MUTE_ALL*/:

		// it's the mute all mux; needs special processing
		dwVal = 	bMute?	((dwVal & ~0x01) | 0x08):	// force EECLK bit to 0
							((dwVal & ~0x08) | 0x01 );	// set EECLK bit to 1: disable output (works as open drain)
		dwRet = PokeBA0 ( pDriverContext,  dwAddr, dwVal );
		break;
	case 4 /*LINE_OUT_MUTE*/:
	case 5 /*RF_OUT_MUTE*/:
	case 6 /*LINE_IN_MIXED_MUTE*/:
	case 7 /*PCM_MUTE*/:
	case 8 /*BTSC_MUTE*/:
	case 9 /*BCM_MIXED_MUTE*/:
	case 10 /*SPDIF_OUT_MUTE*/:

		dwVal  = bMute? (dwVal | MIXDEV_MIXER_MUTE):
						(dwVal & ~MIXDEV_MIXER_MUTE);
		dwRet = PokeBA0 ( pDriverContext,  dwAddr, dwVal );
		break;
	default:
		//error: we received an ID for a control that's not a mute!
		ASSERT(FALSE);
		DEBUGMSG( ZONE_ERROR, (TEXT("error: SetMixerMute: bad ID: %d\r\n"), dwId));
		dwRet = ERROR_NOT_SUPPORTED;
	}

	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("-SetMixerMute: 0x%08x\r\n"), dwRet) );
	return dwRet;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	GetMixerVolume

Description:

	Gets the volume for one of the mixer controls

Arguments:

	PDRIVER_CONTEXT pDriverContext : the context
	DWORD dwId :	the ID of the control
	PDWORD *pdwLeft :	the volume for the left channel
	PDWORD *pdwRight :	the volume for the right channel

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD GetMixerVolume(PDRIVER_CONTEXT pDriverContext, DWORD dwId, DWORD *pdwLeft, DWORD *pdwRight )
{

	DWORD dwRet = NO_ERROR;

	ASSERT(pDriverContext && pdwLeft && pdwRight);

	ASSERT(dwId >= 0 && dwId < MAX_CONTROLS);
	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+GetMixerVolume id %d \r\n"), dwId));

#ifndef USE_HARDWARE_VALUES

	*pdwLeft = HIWORD(g_Controls[dwId].dwLogicalValue);
	*pdwRight = LOWORD(g_Controls[dwId].dwLogicalValue);

#else

	DWORD dwVal, dwRet = NO_ERROR, dwAddr;


	dwAddr = g_Controls[dwId].dwRegister;

	switch (dwId)
	{
	case ZV_PORT_VOLUME:	// that's a 16 bit volume control, and doesn't use a mute flag
		//HrdwGetLongVolume(pDriverContext, dwAddr, MAKELONG(dwLeft, dwRight));
		break;

	case BCM_MIXED_VOLUME:	// the mic input needs special handling
		dwRet = PeekBA0 ( pDriverContext,  dwAddr, &dwVal );

		if( dwRet != NO_ERROR )
		{
			DEBUGMSG( ZONE_ERROR, (TEXT("Error: GetMixerMute PeekBAO read fail: 0x%08x\r\n"), dwRet) );
			return dwRet;
		}
		// the logical range of the mic gain is from +32 dB to -34.5 dB, in steps of 1.5 dB
		// the physical range of the variable amplifier is +12 dB to -34.5 dB
		// we map the logical range to values between 0 (representing +32 dB) and 0x2C
		// (representing -34.5 dB)
		// find out whether the amplifier is on

		*pdwRight = dwVal & MIXDEV_MASTER_GAIN_MAX;
		if (0 == (dwVal & MIXDEV_MIC_AMP_ON))	// amp on?
		{	// no; 
			*pdwRight += MIXDEV_MIC_AMP_GAIN;
			DEBUGMSG(ZONE_TEST|ZONE_VERBOSE, (TEXT("Mic amp is on\r\n")));
		}
		*pdwLeft = *pdwRight;
		break;
	
	case LINE_OUT_VOLUME:
	case RF_OUT_VOLUME:
	case LINE_IN_MIXED_VOLUME:
	case PCM_VOLUME:
	case BTSC_VOLUME:
	case SPDIF_OUT_VOLUME:

		dwRet = PeekBA0 ( pDriverContext,  dwAddr, &dwVal );

		if( dwRet != NO_ERROR )
		{
			DEBUGMSG( ZONE_ERROR, (TEXT("Error: GetMixerMute PeekBAO read fail: 0x%08x\r\n"), dwRet) );
			return dwRet;
		}

		*pdwRight = dwVal & MIXDEV_MASTER_GAIN_MAX;
		*pdwLeft = (dwVal >> 8) & MIXDEV_MASTER_GAIN_MAX;
		break;
	default:
		//error: we received an ID for a control that's not a volume!
		ASSERT(FALSE);
		DEBUGMSG( ZONE_ERROR, (TEXT("error: GetMixerVolume: bad ID: %d\r\n"), dwId));
		dwRet = ERROR_NOT_SUPPORTED;
	}

#endif	// USE_HARDWARE_VALUES
	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST, (TEXT("-GetMixerVolume : dwRet, (%d, %d)\r\n"), *pdwLeft, *pdwRight) );
	return dwRet;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	GetMixerMute

Description:

	

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD dwId :
	BOOL *pbMute :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD GetMixerMute(PDRIVER_CONTEXT pDriverContext, DWORD dwId, BOOL *pbMute )
{
	DWORD dwRet= NO_ERROR;
	ASSERT(pDriverContext && pbMute);
	ASSERT(dwId >= 0 && dwId < MAX_CONTROLS);

	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+GetMixerMute id: %d\r\n"), dwId));

#ifndef USE_HARDWARE_VALUES

	*pbMute = g_Controls[dwId].dwLogicalValue != 0;

#else
	DWORD dwVal, dwRet= NO_ERROR, dwAddr;

	dwAddr = g_Controls[dwId].dwRegister;

	switch (dwId)
	{
	case ZV_PORT_MUTE:
		// It's the ZV port mute; special processing 
		*pbMute = pDriverContext->bZVMute;
		break;
	case LINE_OUT_MUTE_ALL:
	case RF_OUT_MUTE_ALL:
		// it's the mute all mux; needs special processing
		dwRet = PeekBA0 ( pDriverContext,  dwAddr, &dwVal );
		if( dwRet != NO_ERROR )
		{
			DEBUGMSG( ZONE_ERROR, (TEXT("error: GetMixerMute PeekBAO read fail: 0x%08x\r\n"), dwRet) );
			return dwRet;
		}
		// Get EECLK bit 
		*pbMute = (dwVal & 0x01) == 0;	// it's muted when the bit is 0
		break;
	case LINE_OUT_MUTE:
	case RF_OUT_MUTE:
	case LINE_IN_MIXED_MUTE:
	case PCM_MUTE:
	case BTSC_MUTE:
	case BCM_MIXED_MUTE:
	case SPDIF_OUT_MUTE:
		dwRet = PeekBA0 ( pDriverContext,  dwAddr, &dwVal );
		if( dwRet != NO_ERROR )
		{
			DEBUGMSG( ZONE_ERROR, (TEXT("error: GetMixerMute PeekBAO read fail: 0x%08x\r\n"), dwRet) );
			return dwRet;
		}
		*pbMute  = (dwVal & MIXDEV_MIXER_MUTE) != 0;	// muted when bit MIXDEV_MIXER_MUTE is != 0
		break;
	default:
		//error: we received an ID for a control that's not a mute!
		ASSERT(FALSE);
		DEBUGMSG( ZONE_ERROR, (TEXT("error: GetMixerMute: bad ID: %d\r\n"), dwId));
		dwRet = ERROR_NOT_SUPPORTED;
	}
#endif
	
	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("-GetMixerMute: 0x%08x, report %s\r\n"), dwRet, (*pbMute)?TEXT("MUTED"):TEXT("UNMUTED") ) );
	return dwRet;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	GetMuxValue

Description:

	

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD dwControlID :
	PDWORD pdwValue :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD GetMuxValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, PDWORD pdwValue)
{
	// which mux?
	DWORD dwData, dwRet = NO_ERROR;
	ASSERT(pDriverContext && pdwValue);

// My reading of the docs says we should fill in an array of DWORDS with one entry per mux line,
// each DWORD being a boolean, with exactly one set to true, corresponding to the mux selection.

DEBUGMSG(ZONE_ERROR, (TEXT("mxat - GetMuxValue is   W R O N G  !!!!\r\n")));

	
	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+GetMuxValue: mux %d\r\n"), dwControlID));

	switch (dwControlID)
	{
	case 1 /*LINE_OUT_MUX*/:	// line out loop mux
		PeekBA0 ( pDriverContext,  BA0_GPIOR, &dwData );
		DEBUGMSG(ZONE_TEST|ZONE_VERBOSE, (TEXT("GetMuxValue data :  %d\r\n"), dwData));
		*pdwValue = (dwData & 0x04) != 0;
		break;
	case 2 /*RF_OUT_MUX*/:	// rf out mux
		// Get EEDAT bit 
		PeekBA0 ( pDriverContext,  BA0_CFGI, &dwData );
		*pdwValue = (dwData & 0x02) != 0;
		break;
	case 3 /*SPDIF_OUT_MUX*/:	// s/pdif mux
		#pragma message("The S/PDIF mux is incomplete!!!!")
		
		// BUGBUG: need info from CL about setting this!
		
		DEBUGMSG( ZONE_ERROR, (TEXT("GetMuxValue S/PDIF mixer not implemented yet\r\n")) );
		*pdwValue = g_Controls[dwControlID].dwLogicalValue != 0;
		break;
	default:
		ASSERT(FALSE);
		dwRet = ERROR_NOT_SUPPORTED;
		break;
	}
	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("-GetMuxValue: 0x%8.8x, value %d\r\n"), dwRet, *pdwValue));
	return dwRet;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	SetMuxValue

Description:

	

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD dwControlID :
	DWORD dwValue :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD SetMuxValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, DWORD dwValue)
{
	// which mux?
	DWORD dwData, dwRet = NO_ERROR;
	ASSERT(pDriverContext);

// see GetMuxValue
DEBUGMSG(ZONE_ERROR, (TEXT("mxat - SetMuxValue is   W R O N G  !!!!\r\n")));


	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+SetMuxValue: mux %d, value %d\r\n"), dwControlID, dwValue));
	
	switch (dwControlID)
	{
	case 1 /*LINE_OUT_MUX*/:	// line out loop mux
		PeekBA0 ( pDriverContext,  BA0_GPIOR, &dwData );
		dwData = dwValue	? ((dwData & ~0x08) | 0x04 )	// set GPIO bit to 1: mixer through, disable output (works as open drain)
							: ((dwData & ~0x04) | 0x08);	// set GPIO bit to 0: line in directly through, enable output
		PokeBA0 ( pDriverContext, BA0_GPIOR, dwData );
		break;
	case 2 /*RF_OUT_MUX*/:	// rf out mux
		// Set EEDAT bit 
		PeekBA0 ( pDriverContext,  BA0_CFGI, &dwData );

		dwData = dwValue	? ((dwData & ~0x08) | 0x02 )	// set EEDAT bit to 1: mixer through, disable output (works as open drain)
							: ((dwData & ~0x02) | 0x08);	// set EEDAT bit to 0: RF out directly through, enable output
		PokeBA0 ( pDriverContext, BA0_CFGI, dwData );
		break;
	case 3 /*SPDIF_OUT_MUX*/:	// s/pdif mux
		// BUGBUG : not implemented, waiting for info from CL  [CC - 8/6/1999 ]
		DEBUGMSG( ZONE_ERROR, (TEXT("SetMuxValue S/PDIF mixer not implemented yet\r\n")) );
		g_Controls[dwControlID].dwLogicalValue = dwValue;
		break;
	default:
		ASSERT(FALSE);
		dwRet = ERROR_NOT_SUPPORTED;
		break;
	}

	DEBUGMSG( ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("-SetMuxValue: %d\r\n"), dwRet));
	
	return dwRet;
}

DWORD GetUnsignedValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, DWORD *pdwValue)
{
	// which mux?
	DWORD dwRet = NO_ERROR;
	ASSERT(pDriverContext && pdwValue);
	*pdwValue = 0;

	
	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+GetUnsignedValue: ID %d\r\n"), dwControlID));

	switch (dwControlID)
	{
	case 1 /*ZV_PORT_SAMPLERATE*/:	// ZV port sample rate
		*pdwValue = g_Controls[dwControlID].dwLogicalValue;
		break;
	default:
		ASSERT(FALSE);
		dwRet = ERROR_NOT_SUPPORTED;
		break;
	}
	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("GetUnsignedValue: %d, value %d\r\n"), dwRet, *pdwValue));
	return dwRet;
}

DWORD SetUnsignedValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, DWORD dwValue)
{
	// which mux?
	DWORD dwRet = NO_ERROR;
	ASSERT(pDriverContext);
	
	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("+SetUnsignedValue: ID %d, value %d\r\n"), dwControlID, dwValue));

	switch (dwControlID)
	{
	case 1 /*ZV_PORT_SAMPLERATE*/:	// ZV port sample rate
		if (dwValue < MIN_ZV_RATE || dwValue > MAX_ZV_RATE)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT(" error: SetUnsignedValue: invalid value %d\r\n"), dwValue));
			dwRet = ERROR_INVALID_PARAMETER;
		}
		else
		{
			g_Controls[dwControlID].dwLogicalValue = dwValue;
			SetPlaySampleRate(pDriverContext, BA1_ZV_BASE_REGISTER, dwValue);
		}
		break;
	default:
		ASSERT(FALSE);
		dwRet = ERROR_NOT_SUPPORTED;
	}
	DEBUGMSG(ZONE_FUNCTION|ZONE_TEST|ZONE_VERBOSE, (TEXT("-SetUnsignedValue: %d\r\n"), dwRet));
	return dwRet;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	FindMixerLine

Description:

	Returns a pointer to the mixerline struct for the appropriate ID

Arguments:

	DWORD mxLine : the line ID for the line requested

Return Value: LPMIXERLINE
	NULL if the line doesn't exist on the current platform
-------------------------------------------------------------------*/
LPMIXERLINE FindMixerLine(DWORD mxLine)
{
	// this array maps line IDs to indexes in the g_mixerLines array
	static mapLines[MAX_LINES] = 
	{	
		0, //LINE_OUT
		1, //RF_OUT,
		2, //SPDIF_OUT,
		3, //LINE_IN,
		4, //LINE_IN_MIXED,
		5, //PCM,
		6, //SPDIF_IN,
	};
	if (mxLine < LINE_OUT || mxLine >= MAX_LINES) 
		return NULL;
	if (mapLines[mxLine] == -1)
		return NULL;
	return &g_mixerLines[mapLines[mxLine]];
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	HrdwSetVolume

Description:

	This function sets the volume in a mixer device register. 
	It lets users of the wave device control the PCM or ZV volume
	
Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD dwAddress: the offset of the register to set (one of BA0_PLAYBACK_VOLUME 
	or ZV_PORT_VOLUME)
	DWORD dwVolume : Low order word = left volume, Hi order word = right volume.
					 0x0000 = silence; 0xffff = full volume.

Return Value: void
-------------------------------------------------------------------*/
void HrdwSetVolume(
	PDRIVER_CONTEXT pDriverContext, 
	DWORD dwAddress,
	DWORD dwVolume
	)
{
    DWORD lVol, rVol;
	ASSERT(pDriverContext);
	// Low order word = left volume, Hi order word = right volume.
	// 0x0000 = silence; 0xffff = full volume.
	lVol = (dwVolume & 0x0000ffff) >> 1;  // Keep 15 bits left vol
	rVol = (dwVolume & 0xfffe0000) >> 17; // Keep 15 bits right vol.
	
	lVol = ((lVol*lVol) >> 1) & 0x7fff0000;    // Square volume for
	rVol = ((rVol*rVol) >> 17) & 0x00007fff;   // better human response.
	
	// Convert volume to attenuation.
	lVol ^= 0xffff0000;     // Invert volume info & turn on bit 31.
	rVol ^= 0x0000ffff;     // Invert volume info & turn on bit 15.
	
	// Store the new volume in the audio chip.
	PokeBA1 ( pDriverContext,  dwAddress, lVol | rVol);
}


DWORD GetAssociatedMuteID(DWORD dwVolumeControlID)
{
	int i;
	for (i = 0; i < sizeof(g_ControlsPairs)/(2 * sizeof(g_ControlsPairs[0][0])); i++)\
	{
		if (g_ControlsPairs[i][0] == dwVolumeControlID)
			return g_ControlsPairs[i][1];
	}
	DEBUGMSG(ZONE_TEST, (TEXT("Error in GetAssociatedMuteID Id : %d!!!\r\n"), dwVolumeControlID));
	return (DWORD)-1;
}
