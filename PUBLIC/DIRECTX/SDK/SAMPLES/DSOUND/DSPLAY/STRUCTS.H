//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 1999  Microsoft Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
// File: Structs.h
//
// Desc: Waveaudio structures used by DSPlay sample application
//


struct _TFourCC
{
	char	name[4];
	int operator ==(_TFourCC other )
	{
		return (*(unsigned long UNALIGNED *)this) == (*(unsigned long UNALIGNED *)&other);
	}
	int operator !=(_TFourCC other )
	{
		return !((*this)==(other));
	}
};
typedef struct _TFourCC TFourCC;


struct SRiffHeader
{
	TFourCC	RiffCC;
	unsigned long remainingBytes;
};

struct SWaveHeader
{
	TFourCC	WaveCC;
	TFourCC	FmtCC;
	unsigned long ckSize;
	PCMWAVEFORMAT pcmwf;
};

struct SMiscHeader
{
	TFourCC MiscCC;
	unsigned long ckSize;
};

struct SDataHeader
{
	TFourCC DataCC;
	unsigned long dataSize;
	unsigned char data;
};

TFourCC riffCC = { 'R', 'I', 'F', 'F'  };
TFourCC dataCC = { 'd', 'a', 't', 'a'  };
TFourCC waveCC = { 'W', 'A', 'V', 'E'  };
TFourCC fmtCC  = { 'f', 'm', 't', ' '  };

