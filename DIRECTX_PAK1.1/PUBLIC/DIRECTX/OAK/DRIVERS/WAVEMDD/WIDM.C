// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000  Microsoft Corporation
//  
// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @module  widm.c | Implements the WIDM_XXX messages that are passed to the
//          wave audio driver via the <f WAV_IOControl> function.
//
// @xref    <t Wave Input Driver Messages>
//
// -----------------------------------------------------------------------------
#include <wavemdd.h>


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @topic  Wave Input Driver Messages |
//          The wave input driver receives messages via the <f WAV_IOControl>
//          function.
//
//  @xref                          <nl>
//          <m WIDM_ADDBUFFER>     <nl>
//          <m WIDM_CLOSE>         <nl>
//          <m WIDM_GETDEVCAPS>    <nl>
//          <m WIDM_GETNUMDEVS>    <nl>
//          <m WIDM_GETPOS>        <nl>
//          <m WIDM_OPEN>          <nl>
//          <m WIDM_PREPARE>       <nl>
//          <m WIDM_RESET>         <nl>
//          <m WIDM_START>         <nl>
//          <m WIDM_STOP>          <nl>
//          <m WIDM_UNPREPARE>     <nl>
//
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_RESET |
//          The WIDM_RESET message requests a waveform input driver to stop 
//          recording and return all buffers in the input queue to the 
//          client.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_RESET
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInReset> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_RESET message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          For each buffer remaining in the driver's input queue (see 
//          WIDM_ADDBUFFER), the driver should set WHDR_DONE and clear 
//          WHDR_INQUEUE in the dwFlags member of the buffer's WAVEHDR 
//          structure, and also set the structure's dwBytesRecorded member. 
//          Finally, a <m WIM_DATA> callback message should be sent for each 
//          buffer.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WIDM_RESET(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WIDM_RESET");

    LOCK_GSI(WAPI_IN);
    
    try {
        if(gsi[WAPI_IN].bStarted){
            PDD_WaveProc(WAPI_IN, WPDM_STOP, (DWORD) gsi[WAPI_IN].pwh, 0);
            gsi[WAPI_IN].bStarted = FALSE;
        }
    
        MarkAllAsDone(WAPI_IN);
        RemoveCompleteBlocks(WAPI_IN);

        gsi[WAPI_IN].dwBytePosition = 0;
    
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ERRMSG("Error in WAVEDEV Reset: initializing...");
        InitGSI(WAPI_IN);
    }

    UNLOCK_GSI(WAPI_IN);

    FUNC_WMDD("-wdev_WIDM_RESET");
    return(MMSYSERR_NOERROR);
}    




// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_START |
//          The WIDM_START message requests a waveform input driver to begin 
//          recording.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_START
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInStart> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_START message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
// -----------------------------------------------------------------------------
DWORD
wdev_WIDM_START(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WIDM_START");

    LOCK_GSI(WAPI_IN);
    if(!gsi[WAPI_IN].bStarted){
        PDD_WaveProc(WAPI_IN, WPDM_START, (DWORD) gsi[WAPI_IN].pwh, 0);
        gsi[WAPI_IN].bStarted = TRUE;
    }
    UNLOCK_GSI(WAPI_IN);

    FUNC_WMDD("-wdev_WIDM_START");
    return(MMSYSERR_NOERROR);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_STOP |
//          The WIDM_STOP message requests a waveform input driver to stop 
//          recording.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_STOP
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInStop> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_STOP message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          If a buffer in the input queue (see <m WIDM_ADDBUFFER>) has been 
//          partially filled, the driver should treat it as a full buffer and 
//          return it to the client (see <m WIDM_START>). Empty buffers should 
//          remain in the queue.
//
//          If this message is received and recording is already stopped, the 
//          driver should return MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WIDM_STOP(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WIDM_STOP");

    LOCK_GSI(WAPI_IN);
    if(gsi[WAPI_IN].bStarted){
        PDD_WaveProc(WAPI_IN, WPDM_STOP, (DWORD) gsi[WAPI_IN].pwh, 0);
        gsi[WAPI_IN].bStarted = FALSE;
    }
    UNLOCK_GSI(WAPI_IN);

    FUNC_WMDD("-wdev_WIDM_STOP");
    return(MMSYSERR_NOERROR);
}    


