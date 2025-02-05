// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-2000  Microsoft Corporation
//
//  Module Name:
//
//	misc.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------


#include "precomp.h"
#include "toolhal.h"

#ifdef FB16BPP
ulong BitMasks[] = { 0xF800,0x07E0,0x001F };
#else	// FB16BPP
ulong BitMasks[] = { 0,0,0 };
#endif	// FB16BPP


//
//  WaitForNotBusy -
//  inputs:
//  outputs:
//
void IGS5000::WaitForNotBusy()
{
#ifdef ENABLE_ACCELERATION
    unsigned long i;

    for (i = 0; i < 0x1000000; i++) {
	if ((reg_Cop_control & 0x80) == 0) {
	    break;
	}
    }
#else
    return;
#endif
}

//
//  IsBusy -
//  inputs:
//  outputs:
//
int IGS5000::IsBusy()
{
#ifdef ENABLE_ACCELERATION
    return ( ( reg_Cop_control & 0x80 ) != 0 );
#else
    return 0;
#endif
}


#ifdef ENABLE_ACCELERATION
//
//  SelectSolidColor -
//  inputs:
//  outputs:
//
void IGS5000::SelectSolidColor( unsigned long color )
{
    // Set color and mix mode
    WaitForNotBusy();
    reg_Fore_Mix     = (char) Mix_S;
    reg_Fore_Color   = color;

}
#endif

//
//  GetModeInfo -
//  inputs:
//  outputs:
//
SCODE IGS5000::GetModeInfo(
    GPEMode *pMode,
    int modeNo )
{
    if( modeNo<0 || modeNo >= NumModes() ) {
	return E_INVALIDARG;
    }

    *pMode = m_ModeInfo;

    return S_OK;
}

//
//  NumModes -
//  inputs:
//  outputs:
//
int IGS5000::NumModes()
{
    return 1;
}


//
//  DrvGetMasks -
//  inputs:
//  outputs:
//
ULONG *APIENTRY DrvGetMasks(
    DHPDEV     dhpdev)
{
    return BitMasks;
}

//
//  GetPhysicalVideoMemory -
//  inputs:
//  outputs:
//
void IGS5000::GetPhysicalVideoMemory(
    unsigned long *pPhysicalMemoryBase,
    unsigned long *pVideoMemorySize )
{
    *pPhysicalMemoryBase = (unsigned long)m_pLAW;
    *pVideoMemorySize = m_nVideoMemorySize;
}


#if 1
ULONG IGS5000::DrvEscape(
			SURFOBJ *pso,
			ULONG	 iEsc,
			ULONG	 cjIn,
			PVOID	 pvIn,
			ULONG	 cjOut,
			PVOID	 pvOut) {

    TOOL98_CMD *pcmd;
    (TOOL98_DEVADDR*)pvOut;

    DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("Escape functions")));
    if ( (iEsc==ESCAPE_IGST) &&
	 (cjIn>=sizeof(TOOL98_CMD)) ) {
	pcmd = (TOOL98_CMD*) pvIn;
	if (pcmd->dwSignature!=IGST_SIGNATURE) {
	    return 0;
	}
	switch (pcmd->dwCmd) {
	case TOOL98_GETDEVADDR:
	    ((TOOL98_DEVADDR*)pvOut)->dwDevPhysAddr=(DWORD)m_nLAWPhysical;
	    ((TOOL98_DEVADDR*)pvOut)->dwScrAddr=0;
	    ((TOOL98_DEVADDR*)pvOut)->dwOffAddr=0;
	    ((TOOL98_DEVADDR*)pvOut)->dwIOAddr =0;
	    ((TOOL98_DEVADDR*)pvOut)->dwCOPAddr=0;
	    ((TOOL98_DEVADDR*)pvOut)->dwTVAddr =0;
	    return 1;
	default:
	    return 0;
	}
    }
    return 0;
}
#endif
