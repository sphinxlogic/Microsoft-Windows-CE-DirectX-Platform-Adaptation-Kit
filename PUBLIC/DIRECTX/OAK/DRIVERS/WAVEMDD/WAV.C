// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1998  Microsoft Corporation
//  
// -----------------------------------------------------------------------------
#include <wavemdd.h>

// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   WAV Device Interface | Implements the WAVEDEV.DLL device 
//          interface. These functions are required for the device to 
//          be loaded by DEVICE.EXE.
//
// @xref                          <nl>
//          <f WAV_Init>,         <nl>
//          <f WAV_Deinit>,       <nl>
//          <f WAV_Open>,         <nl>
//          <f WAV_Close>,        <nl>
//          <f WAV_Read>,         <nl>
//          <f WAV_Write>,        <nl>
//          <f WAV_Seek>,         <nl>
//          <f WAV_PowerUp>,      <nl>
//          <f WAV_PowerDown>,    <nl>
//          <f WAV_IOControl>     <nl>
//
// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   Designing a Waveform Audio Driver |
//          A waveform audio driver is responsible for processing messages
//          from the Wave API Manager (WAVEAPI.DLL) to playback and record
//          waveform audio. Waveform audio drivers are implemented as 
//          dynamic link libraries that are loaded by DEVICE.EXE The 
//          default waveform audio driver is named WAVEDEV.DLL (see figure). 
//          The messages passed to the audio driver are similar to those 
//          passed to a user-mode Windows NT audio driver (such as mmdrv.dll).
//
//          <bmp blk1_bmp>
//
//          Like all device drivers loaded by DEVICE.EXE, the waveform
//          audio driver must export the standard device functions, 
//          XXX_Init, XXX_Deinit, XXX_IoControl, etc (see 
//          <t WAV Device Interface>). The Waveform Audio Drivers 
//          have a device prefix of "WAV".  
//
//          Driver loading and unloading is handled by DEVICE.EXE and 
//          WAVEAPI.DLL. Calls are made to <f WAV_Init> and <f WAV_Deinit>.
//          When the driver is opened by WAVEAPI.DLL calls are made to 
//          <f WAV_Open> and <f WAV_Close>.  On system power up and power down
//          calls are made to <f WAV_PowerUp> and <f WAV_PowerDown>. All
//          other communication between WAVEAPI.DLL and WAVEDEV.DLL is 
//          done by calls to <f WAV_IOControl>. The other WAV_xxx functions
//          are not used.
//
// @xref                                          <nl>
//          <t Designing a Waveform Audio PDD>    <nl>
//          <t WAV Device Interface>              <nl>
//          <t Wave Input Driver Messages>        <nl>
//          <t Wave Output Driver Messages>       <nl>
//
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Deinit | Device deinitialization routine
//
//  @parm   DWORD | dwData | value returned from WAV_Init call
//
//  @rdesc  Returns TRUE for success, FALSE for failure.
//
// -----------------------------------------------------------------------------
BOOL
WAV_Deinit(
    DWORD dwData
    )
{
    return WMDD_Deinit(dwData);
}



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Init | Device initialization routine
//
//  @parm   DWORD | dwInfo | info passed to RegisterDevice
//
//  @rdesc  Returns a DWORD which will be passed to Open & Deinit or NULL if
//          unable to initialize the device.
//
// -----------------------------------------------------------------------------
DWORD
WAV_Init(
    DWORD Index
    )
{
    return(WMDD_Init(Index));
}



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Open    | Device open routine
//
//  @parm   DWORD | dwData      | Value returned from WAV_Init call (ignored)
//
//  @parm   DWORD | dwAccess    | Requested access (combination of GENERIC_READ
//                                and GENERIC_WRITE) (ignored)
//
//  @parm   DWORD | dwShareMode | Requested share mode (combination of
//                                FILE_SHARE_READ and FILE_SHARE_WRITE) (ignored)
//
//  @rdesc  Returns a DWORD which will be passed to Read, Write, etc or NULL if
//          unable to open device.
//
// -----------------------------------------------------------------------------
DWORD
WAV_Open(
    DWORD dwData, 
    DWORD dwAccess, 
    DWORD dwShareMode
    )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("WAV_Open(0x%X)\r\n"), dwData));
    return 4;
}



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_Close | Device close routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
// -----------------------------------------------------------------------------
BOOL
WAV_Close(
    DWORD dwData
    )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("WAV_Close(0x%X)\r\n"), dwData));
    return TRUE;
}



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Read | Device read routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   LPVOID | pBuf | Buffer to receive data (ignored)
//
//  @parm   DWORD | len | Maximum length to read (ignored)
//
//  @rdesc  Returns 0 always. WAV_Read should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
DWORD
WAV_Read(
    DWORD dwData, 
    LPVOID pBuf, 
    DWORD Len
    )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("WAV_Read(0x%X, 0x%X, %d)\r\n"),
                       dwData, pBuf, Len));

    // Return length read
    return 0;
}




// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Write | Device write routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   LPCVOID | pBuf | Buffer containing data (ignored)
//
//  @parm   DWORD | len | Maximum length to write (ignored)
//
//  @rdesc  Returns 0 always. WAV_Write should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
DWORD
WAV_Write(
    DWORD dwData, 
    LPCVOID pBuf, 
    DWORD Len
    )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("WAV_Write(0x%X, 0x%X, %d)\r\n"),
                  dwData, pBuf, Len));

    // return number of bytes written (or -1 for error)
    return 0;
}



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Seek | Device seek routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   long | pos | Position to seek to (relative to type) (ignored)
//
//  @parm   DWORD | type | FILE_BEGIN, FILE_CURRENT, or FILE_END (ignored)
//
//  @rdesc  Returns -1 always. WAV_Seek should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
DWORD
WAV_Seek(
    DWORD dwData, 
    long pos, 
    DWORD type
    )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("WAV_Seek(0x%X, %d, %d)\r\n"), dwData,
                           pos, type));

    // return an error
    return (DWORD)-1;
}




// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   void | WAV_PowerUp | Device powerup routine
//
//  @comm   Called to restore device from suspend mode.  Cannot call any
//          routines aside from those in the dll in this call.
//
// -----------------------------------------------------------------------------
VOID
WAV_PowerUp(
    VOID
    )
{
    WMDD_PowerHandler(FALSE);
}



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   void | WAV_PowerDown | Device powerdown routine
//
//  @comm   Called to suspend device.  Cannot call any routines aside from
//          those in the dll in this call.
//
// -----------------------------------------------------------------------------
VOID
WAV_PowerDown(
    VOID
    )
{
    WMDD_PowerHandler(TRUE);
}




// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_IOControl | Device IO control routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @parm   DWORD | dwCode | 
//          IO control code for the function to be performed. WAV_IOControl only
//          supports one IOCTL value (IOCTL_WAV_MESSAGE)
//
//  @parm   PBYTE | pBufIn | 
//          Pointer to the input parameter structure (<t MMDRV_MESSAGE_PARAMS>).
//
//  @parm   DWORD | dwLenIn | 
//          Size in bytes of input parameter structure (sizeof(<t MMDRV_MESSAGE_PARAMS>)).
//
//  @parm   PBYTE | pBufOut | Pointer to the return value (DWORD).
//
//  @parm   DWORD | dwLenOut | Size of the return value variable (sizeof(DWORD)).
//
//  @parm   PDWORD | pdwActualOut | Unused
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
//  @xref   <t Wave Input Driver Messages> (WIDM_XXX) <nl>
//          <t Wave Output Driver Messages> (WODM_XXX)
//
// -----------------------------------------------------------------------------
BOOL
WAV_IOControl(
    DWORD  dwOpenData, 
    DWORD  dwCode, 
    PBYTE  pBufIn,
    DWORD  dwLenIn, 
    PBYTE  pBufOut, 
    DWORD  dwLenOut,
    PDWORD pdwActualOut
    )
{
    DWORD dwRet;
    PMMDRV_MESSAGE_PARAMS pParams = (PMMDRV_MESSAGE_PARAMS) pBufIn;

#if 1
    FUNC_WMDD("+WAV_IOControl");
    HEXPARAM(dwOpenData); 
    HEXPARAM(dwCode); 
    HEXPARAM(pBufIn);
    HEXPARAM(dwLenIn); 
    HEXPARAM(pBufOut); 
    HEXPARAM(dwLenOut);
    HEXPARAM(pdwActualOut);
#endif

    //  set the error code to be no error first
    SetLastError(MMSYSERR_NOERROR);

    try {

        if (dwCode != IOCTL_WAV_MESSAGE) {
            PRINTMSG(ZONE_WODM, (TEXT("Unsupported dwCode in WAV_IOControl()\r\n")));
            return(FALSE);
        }

        switch (pParams->uMsg) {

            case WODM_BREAKLOOP:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_BREAKLOOP\r\n")));
                dwRet = wdev_WODM_BREAKLOOP(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_CLOSE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_CLOSE\r\n")));
                dwRet = wdev_COMMON_CLOSE(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_GETDEVCAPS:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_GETDEVCAPS\r\n")));
                dwRet = wdev_COMMON_GETDEVCAPS(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_GETNUMDEVS:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_GETNUMDEVS\r\n")));
                dwRet = wdev_COMMON_GETNUMDEVS(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_GETPITCH:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_GETPITCH\r\n")));
                dwRet = wdev_WODM_GETPITCH(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_GETPLAYBACKRATE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_GETPLAYBACKRATE\r\n")));
                dwRet = wdev_WODM_GETPLAYBACKRATE(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_GETPOS:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_GETPOS\r\n")));
                dwRet = wdev_COMMON_GETPOS(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_GETVOLUME:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_GETVOLUME\r\n")));
                dwRet = wdev_WODM_GETVOLUME(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_OPEN:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_OPEN\r\n")));
                dwRet = wdev_COMMON_OPEN(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_PAUSE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_PAUSE\r\n")));
                dwRet = wdev_WODM_PAUSE(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_PREPARE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_PREPARE\r\n")));
                dwRet = wdev_COMMON_PREPARE(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_RESET:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_RESET\r\n")));
                dwRet = wdev_WODM_RESET(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_RESTART:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_RESTART\r\n")));
                dwRet = wdev_WODM_RESTART(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_SETPITCH:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_SETPITCH\r\n")));
                dwRet = wdev_WODM_SETPITCH(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_SETPLAYBACKRATE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_SETPLAYBACKRATE\r\n")));
                dwRet = wdev_WODM_SETPLAYBACKRATE(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_SETVOLUME:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_SETVOLUME\r\n")));
                dwRet = wdev_WODM_SETVOLUME(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_UNPREPARE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_UNPREPARE\r\n")));
                dwRet = wdev_COMMON_UNPREPARE(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WODM_WRITE:
                PRINTMSG(ZONE_WODM, (TEXT("WODM_WRITE\r\n")));
                dwRet = wdev_WODM_WRITE(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_ADDBUFFER:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_ADDBUFFER\r\n")));
                dwRet = wdev_COMMON_ADDBUFFER(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_CLOSE:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_CLOSE\r\n")));
                dwRet = wdev_COMMON_CLOSE(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_GETDEVCAPS:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_GETDEVCAPS\r\n")));
                dwRet = wdev_COMMON_GETDEVCAPS(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_GETNUMDEVS:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_GETNUMDEVS\r\n")));
                dwRet = wdev_COMMON_GETNUMDEVS(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_GETPOS:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_GETPOS\r\n")));
                dwRet = wdev_COMMON_GETPOS(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_OPEN:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_OPEN\r\n")));
                dwRet = wdev_COMMON_OPEN(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_PREPARE:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_PREPARE\r\n")));
                dwRet = wdev_COMMON_PREPARE(WAPI_IN, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_RESET:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_RESET\r\n")));
                dwRet = wdev_WIDM_RESET(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_START:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_START\r\n")));
                dwRet = wdev_WIDM_START(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_STOP:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_STOP\r\n")));
                dwRet = wdev_WIDM_STOP(pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            case WIDM_UNPREPARE:
                PRINTMSG(ZONE_WIDM, (TEXT("WIDM_UNPREPARE\r\n")));
                dwRet = wdev_COMMON_UNPREPARE(WAPI_OUT, pParams->uDeviceId, pParams->dwUser,
                                         pParams->dwParam1, pParams->dwParam2);
                break;

            default:
                return(PDD_AudioMessage(pParams->uMsg, 
                                        pParams->dwParam1, 
                                        pParams->dwParam2));
        }

        //
        // Pass the return code back via pBufOut
        //
        if (pBufOut != NULL) {
            *((DWORD*) pBufOut) = dwRet;
        }   


    } except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        RETAILMSG(1, (TEXT("EXCEPTION IN WAV_IOControl!!!!\r\n")));
    }   

    return TRUE;
}


