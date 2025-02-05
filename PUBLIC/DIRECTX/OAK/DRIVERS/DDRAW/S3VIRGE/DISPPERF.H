/*******************************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998 Microsoft Corporation

DISPPERF.H

*******************************************************************************/

#ifndef _DISPPERF_H_
#define _DISPPERF_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DO_DISPPERF  
    
#ifdef DISPPERF_DECLARE
#define DISPPERF_EXTERN
#else
#define DISPPERF_EXTERN extern
#endif

typedef struct _DISPPERF_TIMING {
    DWORD   RopCode;
    DWORD   cGPE;
    DWORD   dwGPETime;
    DWORD   cEmul;
    DWORD   dwEmulTime;
    DWORD   cHardware;
    DWORD   dwHardwareTime;
} DISPPERF_TIMING, *PDISPPERF_TIMING;

// The global variables used
DISPPERF_EXTERN DWORD   v_dwDPTimerStart;
#ifdef DISPPERF_DECLARE
DISPPERF_EXTERN DWORD   v_dwCurRop = -1;
#else
DISPPERF_EXTERN DWORD   v_dwCurRop;
#endif
DISPPERF_EXTERN DWORD   v_dwAccelType;
DISPPERF_EXTERN DWORD   v_dwLastRop;
DISPPERF_EXTERN DWORD   v_dwUnhandled;
DISPPERF_EXTERN DWORD   v_dwTimerStart;
#ifndef NUM_ROP_TIMES
#define NUM_ROP_TIMES   32
#endif
DISPPERF_EXTERN DISPPERF_TIMING v_dptData[NUM_ROP_TIMES];

#define DISPPERF_ACCEL_GPE          0
#define DISPPERF_ACCEL_EMUL         1
#define DISPPERF_ACCEL_HARDWARE     2

// Let's define some fake ROP's for line etc.
#define ROP_LINE                0xFEFEFFF1  
#define ROP_MOVE_CURSOR         0xFEFEFFF2
#define ROP_SET_POINTER_SHP     0xFEFEFFF3

// Let's define the ExtEscape codes for this.
#define DISPPERF_EXTESC_GETSIZE         0xFEFEFFF0
#define DISPPERF_EXTESC_GETTIMING       0xFEFEFFF1
#define DISPPERF_EXTESC_CLEARTIMING     0xFEFEFFF2
#define DISPPERF_EXTESC_GETUNHANDLED    0xFEFEFFF3

__inline void DispPerfStart (DWORD RopCode)
{
    LARGE_INTEGER   LargeInt;
        
    // Check to see if we get re-entered
    ASSERT (v_dwCurRop == -1);

    for (v_dwCurRop = 0; v_dwCurRop < NUM_ROP_TIMES; v_dwCurRop++) {
        if (v_dptData[v_dwCurRop].RopCode == RopCode) {
            break;
        }
    }
    if (NUM_ROP_TIMES == v_dwCurRop) {
        // Is there room for another rop?
        for (v_dwCurRop = 0; v_dwCurRop < NUM_ROP_TIMES; v_dwCurRop++) {
            if ((0 == v_dptData[v_dwCurRop].RopCode) &&
                (0 == v_dptData[v_dwCurRop].cGPE) &&
                (0 == v_dptData[v_dwCurRop].cEmul) &&
                (0 == v_dptData[v_dwCurRop].cHardware)) {
                v_dptData[v_dwCurRop].RopCode = RopCode;
                break;
            }
        }
        // No room, Save the last ROP code
        if (NUM_ROP_TIMES == v_dwCurRop) {
            v_dwLastRop = v_dptData[v_dwCurRop].RopCode;
            v_dwCurRop = -1;
        }
    }
    QueryPerformanceCounter (&LargeInt);
    
    v_dwTimerStart = LargeInt.LowPart;
    v_dwAccelType = DISPPERF_ACCEL_GPE;
}

__inline VOID DispPerfType(DWORD Type)
{
    v_dwAccelType = Type;
}
__inline VOID DispPerfEnd(DWORD arg)
{
    LARGE_INTEGER LargeInt;
    DWORD           dwOpTime;
    
    if (v_dwCurRop != -1) {
        QueryPerformanceCounter (&LargeInt);
        dwOpTime = LargeInt.LowPart - v_dwTimerStart;
        switch (v_dwAccelType) {
        case DISPPERF_ACCEL_GPE :
            v_dptData[v_dwCurRop].cGPE++;
            v_dptData[v_dwCurRop].dwGPETime += dwOpTime;
            break;
        case DISPPERF_ACCEL_EMUL :
            v_dptData[v_dwCurRop].cEmul++;
            v_dptData[v_dwCurRop].dwEmulTime += dwOpTime;
            break;
        case DISPPERF_ACCEL_HARDWARE :
            v_dptData[v_dwCurRop].cHardware++;
            v_dptData[v_dwCurRop].dwHardwareTime += dwOpTime;
            break;
        }
        v_dwCurRop = -1;
    }
    
}

__inline int DispPerfQueryEsc(DWORD SupportChk)
{
    if ((SupportChk == DISPPERF_EXTESC_GETSIZE) ||
        (SupportChk == DISPPERF_EXTESC_GETTIMING) ||
        (SupportChk == DISPPERF_EXTESC_CLEARTIMING) ||
        (SupportChk == DISPPERF_EXTESC_GETUNHANDLED))  {
        return 1;
    }
    return 0;
}
__inline int DispPerfDrvEscape (ULONG iEsc, ULONG cjIn, PVOID pvIn,
                               ULONG cjOut, PVOID pvOut)
{
    int RetVal;
    switch (iEsc) {
        case  DISPPERF_EXTESC_GETSIZE:
            if ((cjOut >= sizeof(DWORD)) && (pvOut != NULL)) {
                *(DWORD *)pvOut = sizeof(v_dptData);
                RetVal = 1;
            } else {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
            }
            break;
        case  DISPPERF_EXTESC_GETTIMING:
            if ((cjOut >= sizeof(v_dptData)) && (pvOut != NULL)) {
                memcpy ((char *)pvOut, (char *)v_dptData, sizeof(v_dptData));
                RetVal = 1;
            } else {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
            }
            break;
        case  DISPPERF_EXTESC_CLEARTIMING:
            memset (v_dptData, 0, sizeof(v_dptData));
            RetVal = 1;
            break;
        case  DISPPERF_EXTESC_GETUNHANDLED:
            if ((cjOut >= sizeof(DWORD)) && (pvOut != NULL)) {
                *(DWORD *)pvOut = v_dwUnhandled;
                RetVal = 1;
            } else {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
            }
            break;
        default :
            break;
    }
    return RetVal;
}


#else // DO_DISPPERF

#define DispPerfStart(x)    (0)
#define DispPerfType(x)     (0)
#define DispPerfEnd(x)      (0)
#define DispPerfQueryEsc(x) (0)
#define DispPerfDrvEscape(e,ci,pi,co,po) (0)

#endif // DO_DISPPERF


#ifdef __cplusplus
}
#endif

#endif // ifndef _DISPPERF_H_



