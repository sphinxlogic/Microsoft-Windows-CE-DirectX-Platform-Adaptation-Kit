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

	csmix.h

	Abstract:

	Defines the structures that describe the fake mixer hardware

-------------------------------------------------------------------*/



#ifndef _CSMIX_H_
#define _CSMIX_H_

// three destination lines: line out, RF out and SPDIF out
#define MAX_DESTINATION_LINES	3	

#define MAX_PLATFORMS			2

#define DESTINATION_LINES		3

#define MAX_CONTROLS			23
#define MAX_MULTIPLEXERS		4
#define MAX_MUX_INPUTS			4	// maximum number of inputs for a multiplexer

#define	MAX_ATTENUATION			10000

// some helpful definitions
#define	MONO	1
#define STEREO  2

#define	MUTE	TRUE
#define UNMUTE  FALSE

#define	LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1

// minimum and maximum sample rates accepted for ZV port input
#define MIN_ZV_RATE	2400
#define MAX_ZV_RATE	48000


#define MIXDEV_MIXER_GAIN_MAX		0x001F
#define MIXDEV_MASTER_GAIN_MAX		0x003F
#define MIXDEV_MIXER_MUTE			0x8000
#define MIXDEV_MIC_AMP_ON			0x0080
#define MIXDEV_MIC_AMP_GAIN			0x0D
#define MIXERDRIVER_NAME TEXT("Sample Audio Mixer")


// the lines are:
// three destinations and 7 sources: line in fixed, line in mixed, PCM, BTSC, BCM, BCM mixed, SPDIF in
typedef enum
{
	NO_CONNECTION = -1,
		LINE_OUT = 0,
		RF_OUT,
		SPDIF_OUT,
		// sources
		LINE_IN,
		LINE_IN_MIXED,
		PCM,
		SPDIF_IN,
		// MAX_LINES must always be the last!
		MAX_LINES
}	MIXER_LINES;

typedef struct
{
	DWORD dwMuxID;
	MIXER_LINES muxedLines[MAX_MUX_INPUTS];
} MULTIPLEXER;

typedef struct
{
	DWORD dwLineID;			// the line with which the control is associated
    DWORD dwControlType; 
	DWORD dwLogicalValue;
	
	DWORD  dwRegister;	// the AC97 register; only for VOLUME/MUTE
	DWORD  dwMinimum;	// hardware value corresponding to minimum sound level
	DWORD  dwMaximum;	// hardware value corresponding to maximum sound level
	//
	// 
    TCHAR  szShortName[MIXER_SHORT_NAME_CHARS]; 
    TCHAR  szName[MIXER_LONG_NAME_CHARS]; 
} SHORTMIXERCONTROL, *PSHORTMIXERCONTROL, FAR *LPSHORTMIXERCONTROL; 

extern MIXERLINE g_mixerLines[MAX_LINES];
extern MIXER_LINES g_Connections[MAX_DESTINATION_LINES][MAX_LINES - MAX_DESTINATION_LINES - 1];
extern SHORTMIXERCONTROL g_Controls[MAX_CONTROLS];
extern MULTIPLEXER g_Multiplexers[MAX_MULTIPLEXERS];

// mixer functions

DWORD SetMixerMute(PDRIVER_CONTEXT pDriverContext, DWORD dwId, BOOL bMute);
DWORD SetMixerVolume(PDRIVER_CONTEXT pDriverContext, DWORD dwId, DWORD dwLeft, DWORD dwRight );
DWORD GetMixerVolume(PDRIVER_CONTEXT pDriverContext, DWORD dwId, DWORD *pdwLeft, DWORD *pdwRight );
DWORD GetMixerMute(PDRIVER_CONTEXT pDriverContext, DWORD dwId, BOOL *pbMute );

DWORD GetMuxValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, PDWORD pdwValue);
DWORD SetMuxValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, DWORD dwValue);
DWORD GetUnsignedValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, PDWORD pdwValue);
DWORD SetUnsignedValue(PDRIVER_CONTEXT pDriverContext, DWORD dwControlID, DWORD dwValue);


LPMIXERLINE FindMixerLine(DWORD mxLine);

#endif // _CSMIX_H_

