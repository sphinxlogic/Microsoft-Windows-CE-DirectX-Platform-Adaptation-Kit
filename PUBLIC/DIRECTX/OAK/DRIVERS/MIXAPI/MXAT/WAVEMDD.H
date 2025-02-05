//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************
//
//  Module Name:  
//  
//      wavemdd.h
//  
//  Abstract:  
//  
//  Functions:
//  
//  Notes:
//  
// -----------------------------------------------------------------------------
#ifndef __WAVEMDD_H__
#define __WAVEMDD_H__

//
// Includes common to all the MDD files.
//
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <string.h>
#include <nkintr.h>
#include <excpt.h>
#include <wavedbg.h>
#include <wavedev.h>
#include <waveddsi.h>
#include <mmddk.h>


typedef struct  {
    DWORD  dwOpenData;
    DWORD  dwCode;
    PBYTE  pBufIn;
    DWORD  dwLenIn;
    PBYTE  pBufOut;
    DWORD  dwLenOut;
    PDWORD pdwActualOut;
} IOCTL_PARAMS, *PIOCTL_PARAMS;




typedef struct {
    PWAVEHDR        pwh;
    PWAVEHDR        pwhReal;
    BOOL            bInLoop;
    BOOL            bStarted;
    BOOL            bPaused;            // Is stream paused?
    BOOL            bInMiddle;          // Slightly different than bStarted.
                                        //  used with bPaused.
    DRVCALLBACK*    pfnCallback;
    DWORD           dwOpenFlags;
    PWAVEFORMATEX   pwfx;
    HANDLE          hWave;
    DWORD           dwBytePosition;
	HANDLE hCallerProcess;	// the handle to the process that opened a device
} STREAM_INFO, *PSTREAM_INFO;


extern STREAM_INFO gsi[];

// DEVICE.EXE Driver status.
extern BOOL    g_bDriverInitialized;    //  whether it has been inited or not
extern BOOL    g_bDriverOpened;         // The driver WAV? has been opened.

extern CRITICAL_SECTION v_GSICritSect[];

#define   LOCK_GSI(apidir)  EnterCriticalSection(&(v_GSICritSect[apidir]))
#define UNLOCK_GSI(apidir)  LeaveCriticalSection(&(v_GSICritSect[apidir])) 

//
// dwBytesRecorded is being used to indicate how far output has progressed
//
#define IS_BUFFER_SENT(pwh)      ((pwh)->dwBytesRecorded >= (pwh)->dwBufferLength)
#define IS_BUFFER_FULL(pwh)      ((pwh)->dwBytesRecorded >= (pwh)->dwBufferLength)
#define IS_BUFFER_DONE(pwh)      ((pwh)->dwFlags & WHDR_DONE)
#define IS_BUFFER_PREPARED(pwh)  ((pwh)->dwFlags & WHDR_PREPARED)
#define IS_BUFFER_QUEUED(pwh)    ((pwh)->dwFlags & WHDR_INQUEUE)
#define IS_BUFFER_BEGINLOOP(pwh) ((pwh)->dwFlags & WHDR_BEGINLOOP)
#define IS_BUFFER_ENDLOOP(pwh)   ((pwh)->dwFlags & WHDR_ENDLOOP)
#define IS_BUFFER_INLOOP(pwh)    ((pwh)->reserved)

#define MARK_BUFFER_FULL(pwh)       ((pwh)->dwBytesRecorded = (pwh)->dwBufferLength)
#define MARK_BUFFER_EMPTY(pwh)      ((pwh)->dwBytesRecorded = 0)
#define MARK_BUFFER_NOT_INLOOP(pwh) ((pwh)->reserved = 0)
#define MARK_BUFFER_INLOOP(pwh)     ((pwh)->reserved = 1)
#define MARK_BUFFER_DONE(pwh)       ((pwh)->dwFlags |=  WHDR_DONE)
#define MARK_BUFFER_NOT_DONE(pwh)   ((pwh)->dwFlags &= ~WHDR_DONE)
#define MARK_BUFFER_PREPARED(pwh)   ((pwh)->dwFlags |=  WHDR_PREPARED)
#define MARK_BUFFER_UNPREPARED(pwh) ((pwh)->dwFlags &= ~WHDR_PREPARED)
#define MARK_BUFFER_QUEUED(pwh)     ((pwh)->dwFlags |=  WHDR_INQUEUE)
#define MARK_BUFFER_DEQUEUED(pwh)   ((pwh)->dwFlags &= ~WHDR_INQUEUE)

DWORD
GetBytePosition(
    WAPI_INOUT apidir
    );

VOID
InitGSI(
    WAPI_INOUT apidir
    );


VOID
RemoveCompleteBlocks(
    WAPI_INOUT apidir
    );

VOID
MarkAllAsDone(
    WAPI_INOUT apidir
    );

VOID
MarkAllAsNotInLoop(
    WAPI_INOUT apidir
    );

VOID
MarkFullAsDone(
    WAPI_INOUT apidir
    );


VOID
WAPI_Callback(
    WAPI_INOUT apidir,
    UINT uMsg,
    DWORD dwParam1,
    DWORD dwParam2
    );

VOID
WMDD_PowerHandler(
    BOOL power_down
    );

BOOL
WMDD_Deinit(
    DWORD dwData
    );

DWORD
WMDD_Init(
    DWORD Index
    );


DWORD wdev_COMMON_GETDEVCAPS    (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_GETPOS        (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_OPEN          (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_ADDBUFFER     (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_CLOSE         (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_GETNUMDEVS    (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_PREPARE       (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);
DWORD wdev_COMMON_UNPREPARE     (WAPI_INOUT apidir, UINT  uDeviceId, DWORD dwUser,DWORD dwParam1,DWORD dwParam2);

DWORD wdev_WODM_BREAKLOOP       (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_GETPITCH        (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_GETPLAYBACKRATE (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_GETVOLUME       (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_PAUSE           (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_RESET           (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_RESTART         (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_SETPITCH        (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_SETPLAYBACKRATE (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_SETVOLUME       (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WODM_WRITE           (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

DWORD wdev_WIDM_RESET           (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WIDM_START           (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
DWORD wdev_WIDM_STOP            (UINT uDeviceId, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

BOOL CheckProcess(HANDLE hProcess);

#endif // __WAVEMDD_H__
