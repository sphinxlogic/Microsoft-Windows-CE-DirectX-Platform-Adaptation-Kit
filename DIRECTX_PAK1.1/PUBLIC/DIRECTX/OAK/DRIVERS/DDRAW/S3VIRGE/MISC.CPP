/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-2000  Microsoft Corporation

Module Name:    misc.cpp

Abstract:  Utility routines for S3 Virge access
  
Functions:


Notes: 


--*/

#include "precomp.h"
#define DISPPERF_DECLARE
#include "dispperf.h"

ULONG BitMasks[4];

void 
S3Virge::Unlock(void)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::Unlock\r\n")));

    reg_UNLK_EXSR = 0x06;               // Unlock SR[09..FF]
    reg_LOCK_1 = 0x48;                  // Unlock CR[2D..3F]
    reg_LOCK_2 = 0xA5;                  // Unlock CR[40..FF], CR[36], CR[37]
    reg_SYS_CNFG = reg_SYS_CNFG | 1;    // Enable access to the enhanced
                                        // programming registers
}

void 
S3Virge::Lock(void)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::Lock\r\n")));

    WaitForNotBusy();
    reg_SYS_CNFG = reg_SYS_CNFG & 0xFE;
    reg_LOCK_2 = 0x00;
    reg_LOCK_1 = 0x00;
    reg_UNLK_EXSR = 0x00;
}


// Wait for the S3 FIFO to become available
void 
S3Virge::WaitForNotBusy(void)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS,
        (TEXT("S3Virge::WaitForNotBusy() (accelerated)\r\n")));

#ifdef ENABLE_ACCELERATION
#ifdef DEBUG
    ULONG timeoutValue;
    timeoutValue = GetTickCount() + WAIT_FOR_NOT_BUSY_TIMEOUT;

    do
    {
        if (!S3D_IS_BUSY)     // S3d engine idle
            return;
    }
    while (GetTickCount() < timeoutValue);

    DEBUGMSG(1, (TEXT("ERROR: S3Virge::WaitForNotBusy() timed out\r\n")));
#else // DEBUG
    while (S3D_IS_BUSY);
#endif // DEBUG
#endif // ENABLE_ACCELERATION
}


// Check the state of the S3 FIFO
int 
S3Virge::IsBusy(void)
{
#ifdef ENABLE_ACCELERATION
    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("S3Virge::IsBusy() (accelerated)\r\n")));
    return (S3D_IS_BUSY);
#else
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::IsBusy()\r\n")));
    return 0;
#endif
}


void 
S3Virge::WaitForFIFO(UINT numParameters)
{
#ifdef ENABLE_ACCELERATION
#ifdef DEBUG
    ULONG timeoutValue;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::WaitForFIFO()\r\n")));

    // In production code this check should not be required.
    if (numParameters > 16)
    {
        DEBUGMSG(1, (TEXT("ERROR: WaitForFIFO() Input error, ")
             TEXT("only 16 FIFO slots available!\r\n")));
        numParameters = 16;
    }

    // Check FIFO until it clears or until counter clears.
    timeoutValue = GetTickCount() + WAIT_FOR_FIFO_TIMEOUT;
    do
    {
        if (!S3D_FIFO_HAS_ROOM(numParameters))
            return;
    }
    while (GetTickCount() < timeoutValue);

    DEBUGMSG(1,(TEXT("ERROR: WaitForFIFO() FIFO timed out on S3Virge!\r\n")));
#else
    while (S3D_FIFO_HAS_ROOM(numParameters));
#endif // DEBUG
#endif
}


void 
S3Virge::SelectSolidColor(ULONG color)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::SelectSolidColor()\r\n")));

    WaitForFIFO(1); // one parameter required
    reg_MMA4F4 = color;
}

SCODE 
S3Virge::GetModeInfo(GPEMode *pMode, int modeNo)
{
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("S3Virge::GetModeInfo(modeNo=%d)\r\n"), modeNo));
    
    if (modeNo < 0 || modeNo >= NUMVGAMODES)
        return E_INVALIDARG;

    *pMode = VGAMode[modeNo].gpeModeEx.modeInfo;
    return S_OK;
}


SCODE
S3Virge::GetModeInfoEx(GPEModeEx *pModeEx, int modeNo)
{
    DEBUGMSG(VIRGE_VERBOSE_MSGS,
        (TEXT("S3Virge::GetModeInfoEx(modeNo=%d)\r\n"), modeNo));

    if (modeNo < 0 || modeNo >= NUMVGAMODES)
        return E_INVALIDARG;

    *pModeEx = VGAMode[modeNo].gpeModeEx;
    return S_OK;
}

int 
S3Virge::NumModes(void)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::NumModes()\r\n")));

    return NUMVGAMODES;
}


PULONG APIENTRY 
DrvGetMasks(DHPDEV dhpdev)
{
    S3Virge *pGPE = (S3Virge *)dhpdev;
    GPEModeEx modeEx;

    pGPE->GetModeInfoEx(&modeEx, pGPE->GetPhysicalModeId());

    BitMasks[0] = modeEx.dwRBitMask;
    BitMasks[1] = modeEx.dwGBitMask;
    BitMasks[2] = modeEx.dwBBitMask;
    BitMasks[3] = modeEx.dwAlphaBitMask;

    return BitMasks;
}


void 
S3Virge::GetPhysicalVideoMemory(
    PULONG pPhysicalMemoryBase, 
    PULONG pVideoMemorySize
    )
{
    *pPhysicalMemoryBase = (unsigned long)m_pLAW;
    *pVideoMemorySize = m_nVideoMemorySize;
}

ULONG S3Virge::DrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn,
                          ULONG cjOut, PVOID pvOut)
{
    DWORD   SupportChk;
    int     RetVal = 0; // Default not supported
    
    
    DEBUGMSG (1, (TEXT("S3Virge::DrvEscape(0x%X, %d, %d, 0x%X, %d, 0x%X)\r\n"),
                   pso, iEsc, cjIn, pvIn, cjOut, pvOut));

    switch (iEsc) {
        case QUERYESCSUPPORT :
            if (cjIn == sizeof(DWORD)) 
            {
                SupportChk = *(DWORD *)pvIn;
                if (DispPerfQueryEsc(SupportChk)) 
                {
                    RetVal = 1;
                    break;
                }
                if ((SupportChk == GETVFRAMEPHYSICAL) || 
                    (SupportChk == GETVFRAMELEN)) 
                {
                    RetVal = 1;
                    break;
                }
            } 
            else 
            {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
                break;
            }
            break;
        case GETVFRAMEPHYSICAL :
            if ((cjOut >= sizeof(DWORD)) && (pvOut != NULL)) 
            {
                *(DWORD *)pvOut = m_nLAWPhysical;
                RetVal = 1;
            } 
            else 
            {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
            }
            break;
        case GETVFRAMELEN :
            if ((cjOut >= sizeof(DWORD)) && (pvOut != NULL)) 
            {
                *(DWORD *)pvOut = m_nVideoMemorySize;
                RetVal = 1;
            } 
            else 
            {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
            }
            break;
        default :
            RetVal = DispPerfDrvEscape(iEsc, cjIn, pvIn, cjOut, pvOut);
            break;
    }
    return (ULONG) RetVal;
}

DWORD
S3Virge::WaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pd)
{
    /*
    typedef struct _DDHAL_WAITFORVERTICALBLANKDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL lpDD;               // driver struct
        DWORD                   dwFlags;            // flags
        DWORD                   bIsInVB;            // is in vertical blank
        DWORD                   hEvent;             // event
        HRESUL                  ddRVal;             // return value
        LPDDHAL_WAITFORVERTICALBLANK WaitForVerticalBlank;
                                                    // PRIVATE: ptr to callback
    } DDHAL_WAITFORVERTICALBLANKDATA;
    */

    switch (pd->dwFlags)
    {
        case DDWAITVB_I_TESTVB:

            // If TESTVB, it's just a request for the current vertical blank
            // status:

            if (IN_VBLANK)
                pd->bIsInVB = TRUE;
            else
                pd->bIsInVB = FALSE;

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKBEGIN:

            // If BLOCKBEGIN is requested, we wait until the vertical blank
            // is over, and then wait for the display period to end:

            while (IN_VBLANK);
            while (!IN_VBLANK);

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKEND:

            // If BLOCKEND is requested, we wait for the vblank interval to end:

            while (!IN_VBLANK);
            while (IN_VBLANK);

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKBEGINEVENT:

#ifdef ENABLE_INTR
            // if BLOCKBEGINEVENT is requested, we pass hEvent to
            // interrupt handler. The event will be triggered when
            // the vertical blank begins:
            m_hCtl1VSync = (HANDLE)(pd->hEvent);

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
#else
            return DDHAL_DRIVER_NOTHANDLED;
#endif // ENABLE_INTR
    }

    return DDHAL_DRIVER_NOTHANDLED;
}
